#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "RifleVampireProducerConsumer.h"
#include <cstdio>
#include <AlarmClock.h>


/* Returns microseconds since epoch */
uint64_t timestamp_now() {
   return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
}

const size_t kQSize = 1048576; // 2 ^ 20


void PrintResult(const std::string& text, std::vector<uint64_t>&  data) {
   uint64_t sum = 0; for (auto l : latencies) { sum += l; }

   printf("%s\nPercentile latency numbers in microseconds\n%9s|%9s|%9s|%9s|%9s|%9s|%9s|\n%9ld|%9ld|%9ld|%9ld|%9ld|%9ld|%9lf|\n"
          , text
          , "50th"
          , "75th"
          , "90th"
          , "99th"
          , "99.9th"
          , "Worst"
          , "Average"
          , latencies[(size_t)iterations * 0.5]
          , latencies[(size_t)iterations * 0.75]
          , latencies[(size_t)iterations * 0.9]
          , latencies[(size_t)iterations * 0.99]
          , latencies[(size_t)iterations * 0.999]
          , latencies[latencies.size() - 1]
          , (sum * 1.0) / latencies.size()
         );
}



template < typename Function >
void run_log_benchmark(Function&& f, const std::string& what) {
   AlarmClock<std::chrono::milliseconds> runTimeCheck(2 * 60 * 1000); // 2 minutes
   int iterationCount;
   std::vector < uint64_t > latencies;
   std::vector < uint64_t > success_latencies;
   std::vector < uint64_t > failure_latencies;
   
   size_t countSuccessful = 0;
   size_t amount = 0;
   while(!runTimeCheck.Expired()) {
      ++amount;
      uint64_t begin = timestamp_now();
      success = f(amount);      
      auto delay = end - begin;
      uint64_t end = timestamp_now();
      latencies.push_back(delay);
      if (success) {
        success_latencies.push_back(delay);
      } else {
        failure_latencies.push_back(delay);        
      }
   }

   std::sort(latencies.begin(), latencies.end());
   PrintResult(what + " All Latencies", latencies);

   std::sort(success_latencies.begin(), success_latencies.end());
   PrintResult(what + " Success Latencies", success_latencies);
   
   std::sort(failure_latencies.begin(), failure_latencies.end());
   PrintResult(what + "Failure Latencies", failure_latencies);
}



template < typename Function >
void run_benchmark(Function&& producer, Function&& consumer, const std::string& whatProducer, const std::string& whatConsumer) {
   std::vector<std::thread> threads;
   threads.emplace_back(run_log_benchmark<Function>, std::ref(producer), whatProder);
   threads.emplace_back(run_log_benchmark<Function>, std::ref(consumer), whatConsumer);
  

   for (int i = 0; i < thread_count; ++i) {
      threads[i].join();
   }
}

void print_usage() {
   char const* const executable = "perftest";
   printf("Usage \n1. %s lockfree\n2. %s queuenado\n\n", executable, executable);
}



std::vector<std::string> _received;
struct Consumer {
     CircularFifo<void*, kQSize>& _q;

     Producer(CircularFifo<void*, kQSize>& q) : _q(q) {}
   
     bool Get() {
      void* data = nullptr;
      if (_q.pop(data)) {
        std::string* extracted = reinterpret_cast<std::string*>(data);
        _received.push_back({*extracted});
        delete extracted;
      return true;
     }
     return false;
   }
};




std::vector<std::string> _sent;
struct Producer {
     CircularFifo<void*, kQSize>& _q;
     Producer(CircularFifo<void*, kQSize>& q) : _q(q) {}
   
     bool Push(int data) {
      std::string* dataStr = new std::string(std::to_string(data));
      void* raw = reinterpret_cast<void*>(dataStr);

      if (_q.push(raw)) {
        _sent.push_back({std::to_string(data)});
        return true;
     }
     delete dataStr;
     return false;
   }
};




void Push()




int main(int argc, char* argv[]) {
   CircularFifo<void*, kQSize> q;
   if (argc != 2) {
      print_usage();
      return 0;
   }
      int thread_count = 1;

   if (strcmp(argv[1], "lockfree") == 0) {
      auto producer = createProducer(q);
      auto consumer = createConsumer(q);

      run_benchmark("lock_free", producer, consumer);
}

   // } else if (strcmp(argv[1], "queuenado") == 0) {
   //    auto doneReceiving = std::async(std::launch::async, &Receive);
   //    size_t waited = 0;
   //    while(gWaitToStart.load(std::memory_order_seq_cst) && waited < 20) {
   //       ++waited;
   //       std::this_thread::sleep_for(std::chrono::seconds(1));
   //    }
   //    if (gWaitToStart.load(std::memory_order_seq_cst)) {
   //      std::cerr << "L: " << __LINE__ << " func: " << __FUNCTION__ << " failure to start the test (receiver side)" << std::endl; 
   //    } 
   //    auto queuenado = [](int i, char const * const cstr) {
   //       std::ostringstream oss;
   //       oss  << "Logging " << cstr << i << 0 << 'K' << -42.42;
   //       Send(oss.str());
   //    };
   //    {
   //       for (auto threads : { 1, 2, 3, 4 })
   //          run_benchmark(queuenado, threads, "queuenado");

   //    }                
   //    gKeepRunning.store(false);
   //    auto received = doneReceiving.get();
   //    std::cout << "Number of received entries: " << received << std::endl;

   } else {
      print_usage();
   }


   return 0;
}
