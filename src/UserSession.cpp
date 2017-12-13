/*
 * UserSession.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#include "UserSession.h"
//
//template<bool isServer>
//UserSession<isServer>::UserSession() {
//	this->rid = 0;
//	this->uid = 0;
//	this->pings = 0;
//	this->pongs = 3;
//	this->ws = nullptr;
//}
//
//template<bool isServer>
//UserSession<isServer>::~UserSession() {
//
//}

template<bool isServer>
void UserSession<isServer>::setWS(typename uWS::WebSocket<isServer> *ws) {
	this->ws = ws;
}
