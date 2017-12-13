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
		std::unordered_map<int, std::list<UserSession<uWS::SERVER>*>*>::iterator search =
				_sessions.end();
		if (_sessions.empty()) {
			li = new std::list<UserSession<uWS::SERVER>*>();

			_sessions.insert(
					std::pair<int, std::list<UserSession<uWS::SERVER>*>*>(
							s->rid, li));
		}
		else {
			search = _sessions.find(s->rid);
			if (search == _sessions.end()) {
				li = new std::list<UserSession<uWS::SERVER>*>();

				_sessions.insert(
						std::pair<int, std::list<UserSession<uWS::SERVER>*>*>(
								s->rid, li));
			}
			else {
				li = search->second;
			}
		}

		if (li != nullptr) {
			if (li->empty()) {
				li->push_back(s);
			}
			else {
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

void SessionMgr::foreach(
		std::function<void(UserSession<uWS::SERVER>* s)> func) {
	{
		std::unique_lock<std::mutex> l(_m);
		if (_sessions.empty()) {
			return;
		}

		for (auto v : _sessions) {
			for (auto l : (*v.second)) {
				func(l);
			}
		}
	}
}
