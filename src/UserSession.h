/*
 * UserSession.h
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#ifndef USERSESSION_H_
#define USERSESSION_H_

#include <string>
#include <atomic>

#include <uWS/uWS.h>

template<bool isServer>
class UserSession {
public:
	unsigned int rid;
	unsigned int uid;
	std::string name;
	std::string upsd;
	std::string extra;

public:
	std::atomic<unsigned int> count;

public:
	int pings;
	int pongs;

public:
	UserSession() {
		this->rid = 0;
		this->uid = 0;
		this->pings = 0;
		this->pongs = 3;
		this->ws = nullptr;
	}

	virtual ~UserSession(){

	}

public:
	void setWS(uWS::WebSocket<isServer> *ws);

private:
	uWS::WebSocket<isServer> *ws;
};

#endif /* USERSESSION_H_ */
