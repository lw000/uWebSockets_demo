/*
 * BorkerMessage.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#include "ClientBorkerMessage.h"
#include "TransportData.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

ClientBorkerMessage::ClientBorkerMessage() {

	this->setMessageCallback(
			CC_MSG_CALLBACK_4(ClientBorkerMessage::onMessage, this));

	this->addEvent(MESSAGE_CMD_CONNECTED,
			CC_MSG_CALLBACK_3(ClientBorkerMessage::onConnected, this));
	this->addEvent(MESSAGE_CMD_CHAT,
			CC_MSG_CALLBACK_3(ClientBorkerMessage::onChat, this));
	this->addEvent(MESSAGE_CMD_CHAT_REPLAY,
			CC_MSG_CALLBACK_3(ClientBorkerMessage::onChatReplay, this));
	this->addEvent(MESSAGE_CMD_ERROR,
			CC_MSG_CALLBACK_3(ClientBorkerMessage::onError, this));
}

ClientBorkerMessage::~ClientBorkerMessage() {

}

int ClientBorkerMessage::onMessage(uWS::WebSocket<uWS::CLIENT> *ws, int cmd,
		char *message, size_t length) {
	if (cmd == MESSAGE_CMD_CONNECTED) {

	} else if (cmd == MESSAGE_CMD_CHAT) {

	} else if (cmd == MESSAGE_CMD_CHAT_REPLAY) {

	} else if (cmd == MESSAGE_CMD_ERROR) {

	} else {
		printf("cmd: %d\n", cmd);
	}
	return 0;
}

bool ClientBorkerMessage::onConnected(uWS::WebSocket<uWS::CLIENT> *ws,
		char *message, size_t length) {
	ws_chat_protocol::ws_msg_connected con;
	bool r = con.ParseFromArray(message, length);
	if (r) {
		this->uid = con.uid();

		printf("time: %ld, uid:%d, msg: %s\n", con.time(), con.uid(),
				con.msg().c_str());
	}
	return false;
}

bool ClientBorkerMessage::onChat(uWS::WebSocket<uWS::CLIENT> *ws, char *message,
		size_t length) {
	ws_chat_protocol::ws_msg_chat_request chat;
	bool r = chat.ParseFromArray(message, length);
	if (r) {
		printf("time: %ld, from: %d, to: %d, msg: %s\n", chat.time(),
				chat.from(), chat.to(), chat.msg().c_str());
	}
	return false;
}

bool ClientBorkerMessage::onChatReplay(uWS::WebSocket<uWS::CLIENT> *ws,
		char *message, size_t length) {
	ws_chat_protocol::ws_msg_chat_replay replay;
	bool r = replay.ParseFromArray(message, length);
	if (r) {
		printf("time: %ld, code: %d, msg: %s\n", replay.time(), replay.code(),
				replay.msg().c_str());
	}
	return false;
}

bool ClientBorkerMessage::onError(uWS::WebSocket<uWS::CLIENT> *ws,
		char *message, size_t length) {
	ws_chat_protocol::ws_msg_error merror;
	bool r = merror.ParseFromArray(message, length);
	if (r) {
		printf("time: %ld, msg: %s\n", merror.time(), merror.msg().c_str());
	}
	return false;
}
