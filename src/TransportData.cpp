/*
 * TransportData.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#include "TransportData.h"
#include <string.h>

TransportData::TransportData(TYPE t) {
	this->t = t;
	this->length = 0;
	this->cmd = 0;
	this->message = NULL;
}

TransportData::~TransportData() {
	if (message != NULL) {
		delete[] message;
		message = NULL;
	}
}

int TransportData::getCmd() const {
	return this->cmd;
}

int TransportData::getBufferLength() {
	if (this->t == TYPE::encode) {
		return this->length;
	}
	if (this->t == TYPE::decode) {
		return this->length - sizeof(int);
	}
	return -1;
}

char* TransportData::getBuffer() {
	if (this->t == TYPE::encode) {
		return this->message;
	}
	if (this->t == TYPE::decode) {
		return &this->message[sizeof(int)];
	}

	return NULL;
}

int TransportData::build(int cmd, char* msg, int length) {
	if (this->message != NULL) {
		delete[] this->message;
		this->message = NULL;
	}
	this->cmd = cmd;
	this->message = new char[sizeof(int) + sizeof(char) * length];

	// copy data
	memcpy(this->message, &cmd, sizeof(int));
	memcpy(&this->message[sizeof(int)], msg, length);

	this->length = length + sizeof(int);

	return 0;
}

int TransportData::parse(char* msg, int length) {
	if (this->message != NULL) {
		delete[] this->message;
		this->message = NULL;
	}
	this->length = length;
	this->message = new char[sizeof(char) * length];

	// copy data
	memcpy(&this->cmd, msg, sizeof(int));
	memcpy(this->message, msg, length);

	return 0;
}

