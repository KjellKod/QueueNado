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

/* Returns microseconds since epoch */
uint64_t timestamp_now() {
   return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
}

template < typename Function >
void run_log_benchmark(Function&& f, const std::string& what) {
   int iterations = 100000;
   std::vector < uint64_t > latencies;
   char const* const benchmark = "benchmark";
   for (int i = 0; i < iterations; ++i) {
      uint64_t begin = timestamp_now();
      f(i, benchmark);
      uint64_t end = timestamp_now();
      latencies.push_back(end - begin);
   }
   std::sort(latencies.begin(), latencies.end());
   uint64_t sum = 0; for (auto l : latencies) { sum += l; }
   printf("%s percentile latency numbers in microseconds\n%9s|%9s|%9s|%9s|%9s|%9s|%9s|\n%9ld|%9ld|%9ld|%9ld|%9ld|%9ld|%9lf|\n"
          , what
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
void run_benchmark(Function&& f, int thread_count, const std::string& what) {
   printf("\nThread count: %d\n", thread_count);
   std::vector < std::thread > threads;
   for (int i = 0; i < thread_count; ++i) {
      threads.emplace_back(run_log_benchmark<Function>, std::ref(f), what);
   }
   for (int i = 0; i < thread_count; ++i) {
      threads[i].join();
   }
}

void print_usage() {
   char const* const executable = "perftest";
   printf("Usage \n1. %s lockfree\n2. %s queuenado\n\n", executable, executable);
}


int main(int argc, char* argv[]) {
   if (argc != 2) {
      print_usage();
      return 0;
   }
      int thread_count = 1;

   if (strcmp(argv[1], "lockfree") == 0) {
      CircularFifo<std::string*, 1024> q;

      auto producer = createProducer(q);
      auto consumer = createConsumer(q);

      run_benchmark("lock_free", producer, consumer);


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
