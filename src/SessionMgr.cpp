/*
 * SessionMgr.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: root
 */

#include "SessionMgr.h"

#include <algorithm>

SessionMgr::SessionMgr() {
	// TODO Auto-generated constructor stub

}

SessionMgr::~SessionMgr() {
	// TODO Auto-generated destructor stub
}

void SessionMgr::addSession(UserSession<uWS::SERVER>* s) {
	if (s == nullptr) {
		return;
	}

	std::list<UserSession<uWS::SERVER>*>* li = nullptr;

	{
		std::unique_lock<std::mutex> l(_m);
		if (_sessions.empty()) {
			std::list<UserSession<uWS::SERVER>*>* li = new std::list<
					UserSession<uWS::SERVER>*>();

			_sessions.insert(
					std::pair<int, std::list<UserSession<uWS::SERVER>*>*>(
							s->rid, li));
		}
		else {
			li = _sessions.at(s->rid);
		}

		UserSession<uWS::SERVER>* o = nullptr;
		for (auto l : (*li)) {
			if (l->uid == s->uid) {
				o = l;
				break;
			}
		}

		if (o == nullptr) {
			li->push_back(s);
		}
	}
}

void SessionMgr::removeSession(UserSession<uWS::SERVER>* s) {
	if (s == nullptr) {
		return;
	}

	{
		std::unique_lock<std::mutex> l(_m);
		if (_sessions.empty()) {
			return;
		}

		std::list<UserSession<uWS::SERVER>*>* li = nullptr;

		li = _sessions.at(s->rid);

		if (li == nullptr) {
			return;
		}

		if (li->empty()) {
			return;
		}

		std::remove_if(li->begin(), li->end(),
				[s](UserSession<uWS::SERVER>* o) -> bool {
					return (o->uid == s->uid);
				});
	}
}

void SessionMgr::removeSession(int rid, int uid) {

}
