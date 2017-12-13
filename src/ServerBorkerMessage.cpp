/*
 * ServerBorkerMessage.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#include "ServerBorkerMessage.h"

#include "TransportData.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

ServerBorkerMessage::ServerBorkerMessage() {
	this->setMessageCallback(
			CC_MSG_CALLBACK_4(ServerBorkerMessage::onMessage, this));

//	this->addEvent(MESSAGE_CMD_CONNECTED,
//			CC_MSG_CALLBACK_3(ServerBorkerMessage::onConnected, this));
//	this->addEvent(MESSAGE_CMD_CHAT,
//			CC_MSG_CALLBACK_3(ServerBorkerMessage::onChat, this));
}

ServerBorkerMessage::~ServerBorkerMessage() {

}

int ServerBorkerMessage::onMessage(uWS::WebSocket<uWS::SERVER> *ws, int cmd,
		char *message, size_t length) {
	if (cmd == MESSAGE_CMD_CONNECTED) {
		ws_chat_protocol::ws_msg_connected con;
		bool r = con.ParseFromArray(message, length);
		if (r) {
			printf("%ld,%s\n", con.time(), con.msg().c_str());
		}
	} else if (cmd == MESSAGE_CMD_CHAT) {
		ws_chat_protocol::ws_msg_chat_request chat;
		bool r = chat.ParseFromArray(message, length);
		if (r) {
			printf("time: %ld, from: %d, to: %d, msg: %s\n", chat.time(),
					chat.from(), chat.to(), chat.msg().c_str());

			ws_chat_protocol::ws_msg_chat_replay replay;
			replay.set_code(100);
			replay.set_time(time(NULL));
			replay.set_msg("world");
			replay.set_from(chat.from());
			int len = replay.ByteSize();
			char* buff = new char[len];
			bool r = replay.SerializeToArray(buff, len);
			if (r) {
				TransportData trans(TransportData::TYPE::build);
				int c = trans.buildData(MESSAGE_CMD_CHAT_REPLAY, buff, len);
				if (c == 0) {
					ws->send(trans.getBuffer(), trans.getBufferLength(),
							uWS::OpCode::BINARY);
				}
			}
			delete buff;
		}
	} else {
		printf("cmd: %d\n", cmd);
	}
	return 0;
}

bool ServerBorkerMessage::onConnected(uWS::WebSocket<uWS::SERVER> *ws,
		char *message, size_t length) {

	return false;
}

bool ServerBorkerMessage::onChat(uWS::WebSocket<uWS::SERVER> *ws, char *message,
		size_t length) {

	return false;
}
