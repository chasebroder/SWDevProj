#pragma once
//lang::Cpp

#include "../utils/object.h"
#include "../utils/string.h"
#include "../serial/serial.h"
#include "stringarray.h"
#include "../store/key.h"
#include "../store/value.h"
#include "../network/directory.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * Stub class representing a TCP message. For Serializer demo.
 *
 *@author kierzenka.m@husky.neu.edu & broder.c@husky.neu.edu
 */
class Message : public Object
{
public:
	MsgKind kind_;	// the message kind
	size_t sender_; // the index of the sender node
	size_t target_; // the index of the receiver node
	size_t id_;		// an id t unique within the node

	Message() : Object()
	{
		kind_ = Ack;
		sender_ = 0;
		target_ = 0;
		id_ = 0;
	}

	Message(MsgKind kind, size_t sender, size_t target, size_t id) : Object()
	{
		kind_ = kind;
		sender_ = sender;
		target_ = target;
		id_ = id;
	}

	virtual ~Message() {}

	virtual void serialize(Serializer *s)
	{
		s->write(kind_);
		s->write(sender_);
		s->write(target_);
		s->write(id_);
	}

	virtual void deserialize(Serializer *s)
	{
		kind_ = s->readMsgKind();
		sender_ = s->readSizeT();
		target_ = s->readSizeT();
		id_ = s->readSizeT();
	}

	size_t getTarget()
	{
		return target_;
	}

	size_t getSender()
	{
		return sender_;
	}

	MsgKind getKind()
	{
		return kind_;
	}
};

/**
 * This is the message type that will be sent by a KVStore in response to a GetData Message
 */
class ReplyDataMsg : public Message
{
public:
	//key to be sent in message
	Value *value_;
	Key* key_;

	ReplyDataMsg() : Message() {
		key_ = new Key();
		value_ = new Value();
	}

	ReplyDataMsg(Key* k, Value *v, size_t sender, size_t target) : Message(ReplyData, sender, target, 0)
	{
		value_ = v->clone();
		key_ = k->clone();
	}

	~ReplyDataMsg() {
		delete key_;
		delete value_;
	}

	void serialize(Serializer *s)
	{
		Message::serialize(s);
		key_->serialize(s);
		value_->serialize(s);
	}

	void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete key_;
		key_ = new Key();
		key_->deserialize(s);
		delete value_;
		value_ = new Value();
		value_->deserialize(s);
	}

	Value *getValue()
	{
		return value_;
	}
	
	Key *getKey()
	{
		return key_;
	}

	/**
	 * Check if this reply message equals the given one
	 */
	bool equals(ReplyDataMsg* other)
	{
		if (this == other)
		{
			return true;
		}

		return (value_->equals(other->getValue())) && (key_->equals(other->getKey()));
	}
};

/**
 * This is the message type that will be sent by a KVStore to request data when
 * an Application calls waitAndGet.
 */
class WaitAndGetMsg : public Message
{
public:
	Key *key_;

	WaitAndGetMsg() : Message()
	{
		key_ = new Key();
	}

	WaitAndGetMsg(Key *k, size_t sender, size_t target) : Message(WaitAndGet, sender, target, 0)
	{
		key_ = k->clone();
	}

	~WaitAndGetMsg() {
		delete key_;
	}

	void serialize(Serializer *s)
	{
		Message::serialize(s);
		key_->serialize(s);
	}

	void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete key_;
		key_ = new Key();
		key_->deserialize(s);
	}

	/** Return key from this message */
	Key *getKey()
	{
		return key_;
	}
};

class StatusMsg : public Message
{
public:
	String *msg_; //owned

	StatusMsg() : Message()
	{
		msg_ = new String("");
	}

	StatusMsg(String *msg, size_t sender, size_t target, size_t id) : Message(Status, sender, target, id)
	{
		msg_ = new String(*msg);
	}

	~StatusMsg()
	{
		delete msg_;
	}

	// Inherits from Message
	virtual void serialize(Serializer *s)
	{
		Message::serialize(s);
		msg_->serialize(s);
	}

	// Inherits from Message
	virtual void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		msg_->deserialize(s);
	}
};

/** This message sends a list of connected nodes */
class DirectoryMsg : public Message
{
public:
	size_t port_;
	Directory* dir_; //owned, keep track of clients

	DirectoryMsg() : Message()
	{
		dir_ = new Directory();
	}

	DirectoryMsg(Directory* dir, size_t port, size_t sender, size_t target, size_t id) : Message(Dir, sender, target, id)
	{
		dir_ = dir->clone();
		port_ = port;
	}

	~DirectoryMsg() {
		delete dir_;
	}

	virtual void serialize(Serializer *s)
	{
		Message::serialize(s);
		dir_->serialize(s);
		s->write(port_);
	}

	virtual void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete dir_;
		dir_ = new Directory();
		dir_->deserialize(s);
		port_ = s->readSizeT();
	}

	Directory* getDirectory()
	{
		return dir_;
	}
};

/** Message to register a new node */
class RegisterMsg : public Message
{
public:
	NodeInfo* info_; //owned

	RegisterMsg() : Message()
	{
		String* str = new String("127.0.0.1");
		info_ = new NodeInfo(str, 8080);
		delete str;
	}

	RegisterMsg(String* client, size_t port, size_t sender, size_t target, size_t id) : Message(Register, sender, target, id)
	{
		info_ = new NodeInfo(client, port);
	}

	RegisterMsg(char* client, size_t port, size_t sender, size_t target, size_t id) : Message(Register, sender, target, id)
	{
		String* str = new String(client);
		info_ = new NodeInfo(str, port);
		delete str;
	}

	~RegisterMsg()
	{
		delete info_;
	}

	virtual void serialize(Serializer *s)
	{
		Message::serialize(s);
		info_->serialize(s);
	}

	virtual void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete info_;
		info_ = new NodeInfo();
		info_->deserialize(s);
	}

	String* getClient() {
		return new String(info_->getIPAddr());
	}

	size_t getPort() {
		return info_->getPort();
	}

	NodeInfo* getInfo() {
		return info_;
	}
};

/**
 * This is the message type that will be sent by a KVStore to put a KV pair into another
 */
class PutMsg : public Message
{
public:
	Key *key_;       // owned
	Value *value_;   // owned

	PutMsg() : Message() {
		key_ = new Key();
		value_ = new Value();
	}

	PutMsg(Key *k, Value *v, size_t sender, size_t target) : Message(Put, sender, target, 0)
	{
		key_ = k->clone();
		value_ = v->clone();
	}

	~PutMsg() {
		delete key_;
		delete value_;
	}

	void serialize(Serializer *s)
	{
		Message::serialize(s);
		key_->serialize(s);
		value_->serialize(s);
	}

	void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete key_;
		key_ = new Key();
		key_->deserialize(s);
		delete value_;
		value_ = new Value();
		value_->deserialize(s);
	}

	Value *getValue()
	{
		return value_;
	}

	Key *getKey()
	{
		return key_;
	}
};

/**
 * This is the message type that will be sent by a KVStore to request data for a specific key
 */
class GetDataMsg : public Message
{
public:
	Key *key_;

	GetDataMsg() : Message()
	{
		key_ = new Key();
	}
	GetDataMsg(Key *k, size_t sender, size_t target) : Message(GetData, sender, target, 0)
	{
		key_ = k->clone();
	}

	~GetDataMsg() {
		delete key_;
	}

	void serialize(Serializer *s)
	{
		Message::serialize(s);
		key_->serialize(s);
	}

	void deserialize(Serializer *s)
	{
		Message::deserialize(s);
		delete key_;
		key_ = new Key();
		key_->deserialize(s);
	}

	/** Return key from this message */
	Key *getKey()
	{
		return key_;
	}
};

/** Message informing the receiver that the sender is ready for shutdown */
class DoneMsg : public Message
{
public:
	DoneMsg() : Message() {}

	DoneMsg(size_t sender, size_t target, size_t id) : Message(Done, sender, target, id) {}

	~DoneMsg() {}
};


/** Message acknowledges that a message was received */
class AckMsg : public Message
{
public:
	AckMsg() : Message() {}

	AckMsg(size_t sender, size_t target, size_t id) : Message(Ack, sender, target, id) {}

	~AckMsg() {}
};

/** Message sent by server to initiate teardown of nodes */
class TeardownMsg : public Message
{
	public:
	TeardownMsg() : Message() {}

	TeardownMsg(size_t sender, size_t target, size_t id) : Message(Teardown, sender, target, id) {}

	~TeardownMsg() {}
};
