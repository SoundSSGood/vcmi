/*
 * NetworkConnection.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "NetworkConnection.h"

VCMI_LIB_NAMESPACE_BEGIN

NetworkConnection::NetworkConnection(INetworkConnectionListener & listener, const std::shared_ptr<NetworkSocket> & socket, const std::shared_ptr<NetworkContext> & context)
	: socket(socket)
	, timer(std::make_shared<NetworkTimer>(*context))
	, listener(listener)
{
	socket->set_option(boost::asio::ip::tcp::no_delay(true));

	// iOS throws exception on attempt to set buffer size
	constexpr auto bufferSize = 4 * 1024 * 1024;

	try
	{
		socket->set_option(boost::asio::socket_base::send_buffer_size{bufferSize});
	}
	catch(const boost::system::system_error & e)
	{
		logNetwork->error("error setting 'send buffer size' socket option: %s", e.what());
	}

	try
	{
		socket->set_option(boost::asio::socket_base::receive_buffer_size{bufferSize});
	}
	catch(const boost::system::system_error & e)
	{
		logNetwork->error("error setting 'receive buffer size' socket option: %s", e.what());
	}
}

void NetworkConnection::start()
{
	heartbeat();
	startReceiving();
}

void NetworkConnection::startReceiving()
{
	boost::asio::async_read(*socket,
							readBuffer,
							boost::asio::transfer_exactly(messageHeaderSize),
							[self = shared_from_this()](const auto & ec, const auto & endpoint) { self->onHeaderReceived(ec); });
}

void NetworkConnection::heartbeat()
{
	constexpr auto heartbeatInterval = std::chrono::seconds(10);

	timer->expires_after(heartbeatInterval);
	timer->async_wait( [self = shared_from_this()](const auto & ec)
	{
		if (ec)
			return;

		if (!self->socket->is_open())
			return;

		self->sendPacket({});
		self->heartbeat();
	});
}

void NetworkConnection::onHeaderReceived(const boost::system::error_code & ecHeader)
{
	if (ecHeader)
	{
		listener.onDisconnected(shared_from_this(), ecHeader.message());
		return;
	}

	if (readBuffer.size() < messageHeaderSize)
		throw std::runtime_error("Failed to read header!");

	uint32_t messageSize;
	readBuffer.sgetn(reinterpret_cast<char *>(&messageSize), sizeof(messageSize));

	if (messageSize > messageMaxSize)
	{
		listener.onDisconnected(shared_from_this(), "Invalid packet size!");
		return;
	}

	if (messageSize == 0)
	{
		//heartbeat package with no payload - wait for next packet
		startReceiving();
		return;
	}

	boost::asio::async_read(*socket,
							readBuffer,
							boost::asio::transfer_exactly(messageSize),
							[self = shared_from_this(), messageSize](const auto & ecPayload, const auto & endpoint) { self->onPacketReceived(ecPayload, messageSize); });
}

void NetworkConnection::onPacketReceived(const boost::system::error_code & ec, uint32_t expectedPacketSize)
{
	if (ec)
	{
		listener.onDisconnected(shared_from_this(), ec.message());
		return;
	}

	if (readBuffer.size() < expectedPacketSize)
	{
		throw std::runtime_error("Failed to read packet!");
	}

	std::vector<std::byte> message(expectedPacketSize);
	readBuffer.sgetn(reinterpret_cast<char *>(message.data()), expectedPacketSize);
	listener.onPacketReceived(shared_from_this(), message);

	startReceiving();
}

void NetworkConnection::sendPacket(const std::vector<std::byte> & message)
{
	std::lock_guard<std::mutex> lock(writeMutex);
	std::vector<std::byte> headerVector(sizeof(uint32_t));
	uint32_t messageSize = message.size();
	std::memcpy(headerVector.data(), &messageSize, sizeof(uint32_t));

	bool messageQueueEmpty = dataToSend.empty();
	dataToSend.push_back(headerVector);
	if (message.size() > 0)
		dataToSend.push_back(message);

	if (messageQueueEmpty)
		doSendData();
	//else - data sending loop is still active and still sending previous messages
}

void NetworkConnection::doSendData()
{
	if (dataToSend.empty())
		throw std::runtime_error("Attempting to sent data but there is no data to send!");

	boost::asio::async_write(*socket, boost::asio::buffer(dataToSend.front()), [self = shared_from_this()](const auto & error, const auto & )
	{
		self->onDataSent(error);
	});
}

void NetworkConnection::onDataSent(const boost::system::error_code & ec)
{
	std::lock_guard<std::mutex> lock(writeMutex);
	dataToSend.pop_front();
	if (ec)
	{
		logNetwork->error("Failed to send package: %s", ec.message());
		listener.onDisconnected(shared_from_this(), ec.message());
		return;
	}

	if (!dataToSend.empty())
		doSendData();
}

void NetworkConnection::close()
{
	boost::system::error_code ec;
	socket->close(ec);
	timer->cancel(ec);

	//NOTE: ignoring error code, intended
}

VCMI_LIB_NAMESPACE_END
