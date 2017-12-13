/*
 * ClientBorkerMessage.h
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#ifndef ClientBorkerMessage_H_
#define ClientBorkerMessage_H_

#include "BorkerMessage.h"

#include "ws_command.h"
#include "ws_chat_protocol.pb.h"

#include "UserSession.h"

class ClientBorkerMessage: public BorkerMessage<uWS::CLIENT> {
	int uid;

public:
	ClientBorkerMessage();
	virtual ~ClientBorkerMessage();

public:
	int onMessage(uWS::WebSocket<uWS::CLIENT> *ws, int cmd, char *message,
			size_t length);

public:
	bool onConnected(uWS::WebSocket<uWS::CLIENT> *ws, char *message,
			size_t length);

	bool onChat(uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length);

	bool onChatReplay(uWS::WebSocket<uWS::CLIENT> *ws, char *message,
			size_t length);

	bool onError(uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length);
};

#endif /* ClientBorkerMessage_H_ */
