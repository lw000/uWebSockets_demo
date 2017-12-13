/*
 * BorkerMessage.h
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#ifndef BORKERMESSAGE_H_
#define BORKERMESSAGE_H_

#include <uWS/uWS.h>

#include <functional>
#include <unordered_map>

#define CC_WS_MSG_CALLBACK_3(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)
#define CC_WS_MSG_CALLBACK_4(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, ##__VA_ARGS__)

template<bool isServer>
class BorkerMessage {
	typedef typename std::function<
			bool(uWS::WebSocket<isServer> *ws, char *message, size_t length)> msgHandler;

protected:
	std::function<
			int(uWS::WebSocket<isServer> *ws, int cmd, char *message,
					size_t length)> _onMessageHandler;

public:
	BorkerMessage() {

	}

	virtual ~BorkerMessage() {

	}

public:
	void addEvent(int cmd,
			std::function<
					int(uWS::WebSocket<isServer> *ws, char *message,
							size_t length)> handler) {
		typename std::unordered_map<int, msgHandler>::iterator iter =
				this->_handler.find(cmd);
		if (iter == _handler.end()) {
			this->_handler.insert(std::pair<int, msgHandler>(cmd, handler));
		} else {
			iter->second = handler;
		}
	}

	void dispatch(uWS::WebSocket<isServer> *ws, int cmd, char *message,
			size_t length, bool isBinary) {
		msgHandler cb = nullptr;
		if (!_handler.empty()) {
			cb = this->_handler.at(cmd);
		}

		bool goon = true;
		if (cb != nullptr) {
			goon = cb(ws, message, length);
		}

		if (goon) {
			this->_onMessageHandler(ws, cmd, message, length);
		}
	}

protected:
	void setMessageCallback(
			std::function<
					int(uWS::WebSocket<isServer> *ws, int cmd, char *message,
							size_t length)> handler) {
		this->_onMessageHandler = handler;
	}

private:
	std::unordered_map<int, msgHandler> _handler;
};

#endif /* BORKERMESSAGE_H_ */
