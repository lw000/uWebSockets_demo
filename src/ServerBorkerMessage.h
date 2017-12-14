/*
 * ClientBorkerMessage.h
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#ifndef ServerBorkerMessage_H_
#define ServerBorkerMessage_H_

#include "BorkerMessage.h"

#include "ws_chat_protocol.pb.h"
#include "ws_command.h"
#include "SessionMgr.h"

class ServerBorkerMessage: public BorkerMessage<uWS::SERVER> {
	public:
		SessionMgr session_mgr;

	public:
		ServerBorkerMessage();
		virtual ~ServerBorkerMessage();

	public:
		int onMessage(uWS::WebSocket<uWS::SERVER> *ws, int cmd, char *message,
				size_t length);
		bool onBroadCastMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message,
						size_t length);

	public:
		bool onConnected(uWS::WebSocket<uWS::SERVER> *ws, char *message,
				size_t length);

		bool onChat(uWS::WebSocket<uWS::SERVER> *ws, char *message,
				size_t length);
};

#endif /* ServerBorkerMessage_H_ */
