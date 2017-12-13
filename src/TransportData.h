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
		encode = 0, decode = 1
	};

public:
	TransportData(TYPE t);
	virtual ~TransportData();

public:
	int build(int cmd, char* msg, int length);

public:
	int parse(char* msg, int length);

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
