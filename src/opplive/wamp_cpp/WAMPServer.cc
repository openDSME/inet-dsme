#include <WAMPMessageHandler.h>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include "WAMPServer.h"

#include "Directory.h"

using namespace std;
using namespace boost::filesystem;

WAMPServer::WAMPServer()
: basedir(""), debug(false), port(9002) {
}

WAMPServer::WAMPServer(unsigned int port)
: basedir(""), debug(false), port(port) {
}

void WAMPServer::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
	if(debug) {
		cout << "Message received " << msg->get_payload() << endl;
	}

	handler.receiveMessage(clients.right.at(hdl),msg->get_payload());
}

void WAMPServer::setDebug(bool enable) {
	debug = enable;
}

void WAMPServer::on_http(websocketpp::connection_hdl hdl) {
    	server::connection_ptr con = wserver.get_con_from_hdl(hdl);

	try {
		path request;

		if(basedir == "")
		{
			throw 0;
		}

		string resource = con->get_resource();
		size_t found = resource.find_first_of("?#");
		request = canonical(basedir.string()+resource.substr(0,found)); //will throw exception if file not exists!

		if (request.string().compare(0, basedir.string().length(), basedir.string()) != 0)
		{
			throw 0;
		}

		if(!is_regular_file(request))
		{
			if(is_directory(request))
			{
				request += "/index.html";
			}
		}

		cout << request << endl;
		
		ifstream t(request.string());
		if(!t.is_open())
		{
			throw 0;
		}

		con->set_status(websocketpp::http::status_code::ok);

		std::string str;

		t.seekg(0, std::ios::end);   
		str.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		con->set_body(str);

		string mime = "text/plain";
		string extension = request.extension().string();
		if(extension == ".htm" || extension == ".html") {
			mime = "text/html";
		}
		else if(extension == ".svg") {
			mime = "image/svg+xml";
		}
		else if(extension == ".js") {
			mime = "application/javascript";
		}

		con->replace_header("Content-Type",mime);

		t.close();
	}
	catch(...){
		con->set_body("Request could not be handled");
    		con->set_status(websocketpp::http::status_code::not_found);
	}
}

void WAMPServer::setBaseDir(std::string dir) {
    basedir = canonical(dir); //will throw exception if file not exists!
}


bool WAMPServer::validate(connection_hdl hdl) {
	server::connection_ptr con = wserver.get_con_from_hdl(hdl);

	//std::cout << "Cache-Control: " << con->get_request_header("Cache-Control") << std::endl;

	const std::vector<std::string> & subp_requests = con->get_requested_subprotocols();
	std::vector<std::string>::const_iterator it;

	for (it = subp_requests.begin(); it != subp_requests.end(); ++it) {
		std::cout << "Requested: " << *it << std::endl;
	}

	if (subp_requests.size() > 0) {
		con->select_subprotocol(subp_requests[0]);
	}

	return true;
}

string WAMPServer::generateRandomString()
{
	stringstream ss;
	string chars(
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"1234567890");
	random_device rng;
	uniform_int_distribution<> index_dist(0, chars.size() - 1);
	for(int i = 0; i < 16; ++i) {
		ss << chars[index_dist(rng)];
	}
	return ss.str();
}

void WAMPServer::on_open(connection_hdl hdl) {

	connections.insert(hdl);
	try {
		string sessionId = generateRandomString();
		stringstream ss;
		clients.insert(boost::bimap<std::string, connection_hdl>::value_type(sessionId,hdl));
		ss << "[0, \"" << sessionId << "\", 1, \"wamp_cpp/0.0.0.1\"]";
		wserver.send(hdl, ss.str(), websocketpp::frame::opcode::text);
		Directory::getInstance().connectionEstablished(sessionId);
	} catch (const websocketpp::lib::error_code& e) {
		std::cout << "Open connection failed because: " << e  
			<< " (" << e.message() << ")" << std::endl;
	}
}

void WAMPServer::on_close(connection_hdl hdl) {
	connections.erase(hdl);
}

void WAMPServer::send(std::string client, std::string msg)
{
	if(debug) {
		cout << "Send message " << msg << endl;
	}

	try {
		auto hdl = clients.left.at(client);
		if(!hdl.lock().get())
		{
			cout << "Session to " << client << " finished" << endl;
			clients.left.erase(client);
			return;
		}

		wserver.send(hdl, msg, websocketpp::frame::opcode::text);
	} catch (const websocketpp::lib::error_code& e) {
		std::cout << "Send failed because: " << e  
			<< " (" << e.message() << ")" << std::endl;
	} catch(std::out_of_range e) {
		// does not matter -> don't send anything
	}
}

void WAMPServer::start()
{
	serverThread = std::thread(&WAMPServer::thread,this);
}

void WAMPServer::stop()
{
	wserver.stop();
	serverThread.join();
}

void WAMPServer::thread()
{
	handler.registerSend(bind(&WAMPServer::send,this,::_1,::_2));

	try {
		// Set logging settings
		//wserver.set_access_channels(websocketpp::log::alevel::all);
		wserver.clear_access_channels(websocketpp::log::alevel::all);

		// Initialize ASIO
		wserver.init_asio();

		// Register our message handler
		wserver.set_message_handler(bind(&WAMPServer::on_message,this,::_1,::_2));
		wserver.set_validate_handler(bind(&WAMPServer::validate,this,::_1));
		wserver.set_open_handler(bind(&WAMPServer::on_open,this,::_1));
		wserver.set_close_handler(bind(&WAMPServer::on_close,this,::_1));
		wserver.set_http_handler(bind(&WAMPServer::on_http,this,::_1));

		// Listen on port
		wserver.listen(boost::asio::ip::tcp::v4(), port);

		// Start the server accept loop
		wserver.start_accept();

		// Start the ASIO io_service run loop
		wserver.run();

		std::cout << "WAMPServer stopped" << std::endl;
	} catch (const std::exception & e) {
		std::cerr << "WAMPServer error " << e.what() << std::endl;
	} catch (websocketpp::lib::error_code e) {
		std::cerr << "WAMPServer error " << e.message() << std::endl;
	} catch (...) {
		std::cerr << "WAMPServer error unknown exception" << std::endl;
	}
}

