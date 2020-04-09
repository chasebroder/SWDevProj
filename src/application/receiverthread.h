//lang:Cpp

#include "../utils/thread.h"
#include "../network/inetwork.h"
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
    INetwork *network_;
    KVStore *kv_;

    ReceiverThread(size_t node, INetwork *net, KVStore *kv) : Thread()
    {
        nodeNum_ = node;
        network_ = net;
        kv_ = kv;
    }

    ~ReceiverThread() {}

    void run()
    {
        bool isDone = false; //listens for messages as long as still true
        while (!isDone)
        {
			Message *m = network_->receiveMsg(); // blocks until new message arrives
            MsgKind kind = m->getKind();
            fprintf(stderr, "Node %zu received message of kind: %d\n", nodeNum_, kind);
            switch (kind)
            {
            case MsgKind::GetData:
            {
                GetDataMsg *gdMsg = dynamic_cast<GetDataMsg *>(m);
                size_t sender = gdMsg->getSender();
				// Respond with data, nullptr if we don't have it right now
				Value* val = kv_->getValue(gdMsg->getKey());
				ReplyDataMsg *reply = new ReplyDataMsg(gdMsg->getKey(), val, nodeNum_, sender);
				network_->sendMsg(reply);
				delete gdMsg;
                break;
            }
			case MsgKind::WaitAndGet:
            {
                WaitAndGetMsg *wagMsg = dynamic_cast<WaitAndGetMsg *>(m);
                size_t sender = wagMsg->getSender();
                // Respond with data, add it to the queue if we don't have it yet
                Value* val = kv_->getValue(wagMsg->getKey());
                if (val) {
                    ReplyDataMsg *reply = new ReplyDataMsg(wagMsg->getKey(), val, nodeNum_, sender);
                    network_->sendMsg(reply);
                    delete wagMsg;
                } else {
                    kv_->addMsgWaitingOn(wagMsg);
                }
                break;
            }
            case (MsgKind::ReplyData):
            {
				ReplyDataMsg* rdMsg = dynamic_cast<ReplyDataMsg*>(m);
				kv_->addReply(rdMsg);
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
            case (MsgKind::Register):
            {
                RegisterMsg* rMsg = dynamic_cast<RegisterMsg*>(m);
                assert(rMsg);
                network_->handleRegisterMsg(rMsg);
                break;
            }
            case (MsgKind::Dir):
            {
                DirectoryMsg* dirMsg = dynamic_cast<DirectoryMsg*>(m);
                network_->handleDirectoryMsg(dirMsg);
                break;
            }
            case (MsgKind::Done):
            {
                DoneMsg* doneMsg = dynamic_cast<DoneMsg*>(m);
                network_->handleDoneMsg(doneMsg);
                break;
            }
            case (MsgKind::Teardown):
            {
                isDone = true;
                break;
            }
            default:
                pln("Weird msg type...");
            }
			
        }
        printf("End of receive thread run\n");
    }
};