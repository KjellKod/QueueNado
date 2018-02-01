#include <Vampire.h>
#include <Rifle.h>


thread_local std::atomic<bool> gKeepRunning = {true};
std::atomic<bool> gWaitToStart = {true};

std::string gQueueNadoLocation = "ipc:///tmp/perftest.ipc";
Rifle* CreateSendQueue() {
   Rifle* serverQueue = new Rifle(gQueueNadoLocation.c_str());
   serverQueue->SetOwnSocket(false);
   int size = 1000; 
   serverQueue->SetHighWater(size);
   if (!serverQueue->Aim()) {
      delete serverQueue;
      serverQueue = nullptr;
      std::cerr << "ERROR : " << __FUNCTION__ << " L: " << __LINE__ << std::endl;
      return nullptr;
   }
   return serverQueue;
}


void Send(std::string message) {
   thread_local std::unique_ptr<Rifle> sender(CreateSendQueue());
   if (sender == nullptr) {
      std::cerr << "ERROR : " << __FUNCTION__ << " L: " << __LINE__ << std::endl;
      return;
   }

   void* data = nullptr;
   std::string* copymsg = new std::string(message);
   data = reinterpret_cast<void*>(copymsg);
   sender->FireStake(data, 0);
}



size_t Receive() {
   Vampire receiveQ (gQueueNadoLocation.c_str());
   receiveQ.SetOwnSocket(true);
   receiveQ.SetHighWater(1024);
   size_t kMaxWait = 20;
   size_t waitCounter = 0;
   if (!receiveQ.PrepareToBeShot()) {
      std::cerr << "cannot start receiver on " << "ipc:///tmp/nanologtest.ipc" << std::endl;
      return 0;
   }

   size_t received = 0;
   gWaitToStart.store(false, std::memory_order_seq_cst);
   while (gKeepRunning.load() && waitCounter < 20) {
      void* data = nullptr;
      receiveQ.GetStake(data, 0);
      if (data != nullptr) {
         ++received;
         std::string* str = reinterpret_cast<std::string*>(data);
         delete str;
      } else {
        ++waitCounter;
      }
   }
   return received;
}




