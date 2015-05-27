/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 */

// correct way to include ndn-cxx headers
// #include <ndn-cxx/face.hpp>
#include "face.hpp"
#include <time.h>
#include <cstdlib>
#include <sys/time.h>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
timeval now;
int PACKET_SIZE = 1;
double totaltime = 0;
int Packet_NUM =  1;
char *namePrefix = new char[512];
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {
class Consumer : noncopyable
{
public:
	int count = 0;
	int timeoutcount = 0;
  void
  run()
  {	
	for(int i = 0; i <  Packet_NUM  ; i++){
		Name interestName = Name(namePrefix);
		interestName.appendSegment(i);
		Interest interest(interestName);
		
		interest.setInterestLifetime(time::milliseconds(1000000));
		interest.setMustBeFresh(true);

		m_face.expressInterest(interest, bind(&Consumer::onData, this,  _1, _2), bind(&Consumer::onTimeout, this, _1));
		//std::cout << "Sending " << interest << std::endl;
	}	

    // processEvents will block until the requested data received or timeout occurs
	timeval start, finish;
	gettimeofday(&start, 0);
	std::cout << "sending interests at:\t" << start.tv_sec << "s."<<start.tv_usec<<"us"<<std::endl;
	m_face.processEvents();
//	std::cout << "received: " << counttt <<"data packets " << std::endl;
	gettimeofday(&finish, 0);
	std::cout << "finishing data at:\t" << finish.tv_sec << "s."<<finish.tv_usec<<"us"<<std::endl;
	std::cout << "total on network:\t" << (finish.tv_sec - start.tv_sec)* 1000 + (double)(finish.tv_usec - start.tv_usec) / 1000.0 << " ms\n";
	std::cout << "transmission rate:\t" << (double)(  Packet_NUM *PACKET_SIZE ) / ((finish.tv_sec - start.tv_sec) + (double)(finish.tv_usec - start.tv_usec) / 1000000) << " KB/s \n";
	totaltime += ((finish.tv_sec - start.tv_sec) + (double)(finish.tv_usec - start.tv_usec) / 1000000);
  }

private:
  void
  onData(const Interest& interest, const Data& data)
  {
	count++;
//   	std::cout << data << std::endl;
  }

  void
  onTimeout(const Interest& interest)
  {
	timeoutcount++;
	m_face.expressInterest(interest, bind(&Consumer::onData, this,  _1, _2), bind(&Consumer::onTimeout, this, _1));
    //std::cout << "Timeout " << interest << std::endl;
  }

private:
  Face m_face;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
	std::cout << argc <<std::endl;
	namePrefix = *(argv+1);
	PACKET_SIZE = atoi(*(argv + 2));
	Packet_NUM = atoi(*(argv + 3));
	std::cout << "\n\n-------------------  test  ----------------\n";
	gettimeofday(&now, 0);
	std::cout << "packet size is:\t\t" << PACKET_SIZE << " KB\n";
	std::cout << "program start at:\t" << now.tv_sec << "s."<<now.tv_usec<<"us"<<std::endl;
  ndn::examples::Consumer consumer;
  try {
	for(int i = 0; i <1; i++){
	    consumer.run();
	}
	std::cout << "total timeout packet:\t" << consumer.timeoutcount << std::endl;
	std::cout << "total packet received:\t" << consumer.count << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
	gettimeofday(&now, 0);
	std::cout << "program ends at:\t" << now.tv_sec << "s."<<now.tv_usec<<"us"<<std::endl;
  }
	std::cout << "\ntotal network time is:\t" << totaltime << " s\n" << std::endl;
  return 0;
}
