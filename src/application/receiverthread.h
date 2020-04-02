//lang:Cpp

#include "../utils/thread.h"
#include "../network/pseudo/pseudonetwork.h"
#include "../network/message.h"
#include <assert.h>

/**
 * This class represents the receiver thread. It will listen for messages continuously
 * and handle them accordingly 
 * 
 * @authors: broder.c@husky.neu.edu and kierzenka.m@husky.neu.edu
 */
class ReceiverThread : public Thread
{
public:
    size_t nodeNum_;
    PseudoNetwork *network_;
    KVStore *kv_;
	MessageQueue* msgsInProg_; //messages we should deal with (keys we don't have yet)

    ReceiverThread(size_t node, PseudoNetwork *net, KVStore *kv) : Thread()
    {
        nodeNum_ = node;
        network_ = net;
        kv_ = kv;
		msgsInProg_ = new MessageQueue();
		kv_->setBackChannel(msgsInProg_);
    }

    ~ReceiverThread() {}

    void run()
    {
        while (true)
        {
			Message *m = network_->receiveMsg(nodeNum_); // blocks until new message arrives
            MsgKind kind = m->getKind();
            switch (kind)
            {
            case MsgKind::GetData:
            {
                GetDataMsg *gdMsg = dynamic_cast<GetDataMsg *>(m);
                size_t sender = gdMsg->getSender();
				// Respond with data, nullptr if we don't have it right now
				Value* val = kv_->getValue(gdMsg->getKey());
				ReplyDataMsg *reply = new ReplyDataMsg(val, nodeNum_, sender);
				network_->sendMsg(reply);
				delete gdMsg;
                break;
            }
			case MsgKind::WaitAndGet:
            {
                // respond with data
                WaitAndGetMsg *wagMsg = dynamic_cast<WaitAndGetMsg *>(m);
                size_t sender = wagMsg->getSender();
                // Respond with data, add it to the queue if we don't have it right now
                Value* val = kv_->getValue(wagMsg->getKey());
                if (val) {
                    ReplyDataMsg *reply = new ReplyDataMsg(val, nodeNum_, sender);
                    network_->sendMsg(reply);
                    delete wagMsg;
                } else {
                    size_t t = msgsInProg_->size();
                    msgsInProg_->push(wagMsg);
                    assert(msgsInProg_->size() == (t + 1));
                }
                break;
            }
            case (MsgKind::ReplyData):
            {
                kv_->receivedMsgs_->push(m);
                break;
            }
			case (MsgKind::Put):
			{
				PutMsg *msg = dynamic_cast<PutMsg *>(m);
				Key* k = msg->getKey();
				assert(k->getNode() == nodeNum_);
				kv_->put(k, msg->getValue());
                break;
			}
            default:
                pln("Weird msg type...");
            }
			
        }
        
        printf("End of receive thread run\n");
    }
};