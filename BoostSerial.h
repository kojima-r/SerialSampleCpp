#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/steady_timer.hpp>
#include <vector>
#include <boost/chrono.hpp>
class BoostSerial
{
public:
	typedef std::vector<uint8_t> frame_type;
private:
	bool isFinished;
	bool isTimeout;
	unsigned int timeout_sec;
	std::string port;
	int baudrate;
	std::size_t transferred;
	frame_type recvdata;
	boost::asio::io_service& io;
	boost::asio::serial_port sp;
	boost::asio::steady_timer timer;
	boost::system::error_code err;
	boost::asio::streambuf sbuf;
	void WriteHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
	{
		isFinished = true;
		if(ec == boost::asio::error::operation_aborted)
		{
			isTimeout = true;
		}
		else
		{
			isTimeout = false;
			timer.cancel();
		}
		return;
	};
	void ReadHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
	{
		transferred = bytes_transferred;
		isFinished = true;
		if(ec == boost::asio::error::operation_aborted)
		{
			isTimeout = true;
		}
		else
		{
			isTimeout = false;
			timer.cancel();

			const uint8_t* buffer_ptr = boost::asio::buffer_cast<const uint8_t*>(sbuf.data());
			recvdata.assign(buffer_ptr, buffer_ptr+bytes_transferred);
			sbuf.consume(bytes_transferred);
		}
		return;
	};
	void on_timer(const boost::system::error_code& ec)
	{
		sp.cancel();
	};
public:
	BoostSerial(boost::asio::io_service& ios)
		:sp(ios), io(ios), timer(ios), isFinished(false), isTimeout(false), timeout_sec(5)
	{};
	void setPort(const std::string PORT)
	{
		port = PORT;
		return;
	}
	void setBaudrate(int baudrate)
	{
		this->baudrate=baudrate;
		return;
	}
	void setBaudrate(const std::string&  sbaudrate)
	{
		try
		{
			std::stringstream(sbaudrate) >> baudrate;
		}
		catch(std::exception& e)
		{
			baudrate = 9600;
		}
		return;
	}
	bool open(void)
	{
		if(sp.is_open())  return false;
		else
		{
			sp.open(port, err);
			if(err)
			{
				return false;
			}
			else
			{
				unsigned int baudrate = 0;

				sp.set_option(boost::asio::serial_port_base::baud_rate(baudrate), err);
				sp.set_option(boost::asio::serial_port_base::character_size(boost::asio::serial_port_base::character_size::character_size(8)), err);
				sp.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none), err);
				sp.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none), err);
				sp.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one), err);
			}
			return true;
		}
	};
	bool close(void)
	{
		if(!sp.is_open()) return false;
		else
		{
			sp.cancel(err);
			if(err)
				return false;
			else
			{
				sp.close(err);
				if(err)
					return false;
				else
					return true;
			}
		}
	};
	void write(const frame_type& data)
	{
		boost::asio::async_write(
			sp, 
			boost::asio::buffer(data,data.size()),
			boost::asio::transfer_all(),
			boost::bind(&BoostSerial::WriteHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
		boost::system::error_code ec;
		timer.expires_from_now(boost::chrono::seconds(timeout_sec),ec);
		timer.async_wait(boost::bind(&BoostSerial::on_timer, this, boost::asio::placeholders::error));
		isFinished = false;

		io.reset();
		io.run();

		return;
	};
	void write(const frame_type& data, const std::size_t length)
	{
		boost::asio::async_write(
			sp, 
			boost::asio::buffer(data,data.size()),
			boost::asio::transfer_exactly( ( (length < data.size())?length:data.size() ) ),
			boost::bind(&BoostSerial::WriteHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
		boost::chrono::milliseconds to(timeout_sec);
		timer.expires_from_now(to);
		timer.async_wait(boost::bind(&BoostSerial::on_timer, this, boost::asio::placeholders::error));
		isFinished = false;

		io.reset();
		io.run();

		return;
	};
	void read(void)
	{
		boost::asio::async_read(
			sp,
			sbuf,
			boost::asio::transfer_at_least(4),
			boost::bind(&BoostSerial::ReadHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
		timer.expires_from_now(boost::chrono::seconds(timeout_sec));
		timer.async_wait(boost::bind(&BoostSerial::on_timer, this, boost::asio::placeholders::error));
		isFinished = false;

		io.reset();
		io.run();

		return;
	}
	void read(const std::size_t length)
	{
		sbuf.prepare(length);

		boost::asio::async_read(
			sp,
			sbuf,
			boost::asio::transfer_exactly(length),
			boost::bind(&BoostSerial::ReadHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
		timer.expires_from_now(boost::chrono::seconds(timeout_sec));
		timer.async_wait(boost::bind(&BoostSerial::on_timer, this, boost::asio::placeholders::error));
		isFinished = false;

		io.reset();
		io.run();

		return;
	}
	frame_type GetRecvData(void)
	{
		if(isFinished)
			return recvdata;
		else
			return frame_type();
	};
	bool isProcessFinished(void) const
	{
		return isFinished;
	};
	bool isProcessTimeout(void) const
	{
		return isTimeout;
	};
	std::string GetLastErrorMessage(void) const
	{
		return err.message();
	};
	int setTimeout_sec(const unsigned int new_timeout_sec)
	{
		unsigned int prev = timeout_sec;
		timeout_sec = new_timeout_sec;

		return prev;
	}
};