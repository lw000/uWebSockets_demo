/*
 * SessionMgr.h
 *
 *  Created on: Dec 13, 2017
 *      Author: root
 */

#ifndef SESSIONMGR_H_
#define SESSIONMGR_H_

#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>
#include <uWS/uWS.h>

#include "UserSession.h"

class SessionMgr {
	public:
		SessionMgr();
		virtual ~SessionMgr();

	public:
		void addSession(UserSession<uWS::SERVER>* s);
		void removeSession(UserSession<uWS::SERVER>* s);
		void removeSession(int rid, int uid);

	private:
		std::unordered_map<int, std::list<UserSession<uWS::SERVER>*>*> _sessions;
		std::mutex _m;
};

#endif /* SESSIONMGR_H_ */
