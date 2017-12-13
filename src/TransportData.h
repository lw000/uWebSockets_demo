/*
 * TransportData.h
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#ifndef TRANSPORTDATA_H_
#define TRANSPORTDATA_H_

class TransportData {
public:
	enum TYPE {
		build = 0, parse = 1
	};

public:
	TransportData(TYPE t);
	virtual ~TransportData();

public:
	int buildData(int cmd, char* msg, int length);

public:
	int parseData(char* msg, int length);

public:
	int getCmd() const;
	char* getBuffer();
	int getBufferLength();

private:
	TYPE t;
	int cmd;
	char* message;
	int length;
};

#endif /* TRANSPORTDATA_H_ */
