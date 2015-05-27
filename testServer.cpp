#include <iostream>
#include <string>
#include <fstream>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <cstdlib>
#include <algorithm>
#include <time.h>

char *namePrefix  = new char[256];
const int MAX_FILE_SIZE = 64*1024*1024;
size_t xxx = 0;

std::string itoa(int value, int base) {
	enum { kMaxDigits = 35 };
	std::string buf;
	buf.reserve( kMaxDigits ); // Pre-allocate enough space.
	if (base < 2 || base > 16) return buf;
	int quotient = value;
	do {
		buf += "0123456789abcdef"[ std::abs( quotient % base ) ];
		quotient /= base;
	} while ( quotient );
	
	if ( value < 0 && base == 10) buf += '-';
	std::reverse( buf.begin(), buf.end() );
	return buf;
	
}

int count = 0;
namespace ndn{
	class NdnServer:noncopyable{
	public:
		size_t m_TotalSegNum;
		size_t m_ContentSize;
		size_t m_SegmentSize;
		size_t m_FileSize;
		size_t m_TotalPacketNum;
		char *fileBuffer = new char[MAX_FILE_SIZE];

		NdnServer(std::string fileName, size_t segSize, size_t contentsize, int signORnot){
			using namespace std;
			isSign = signORnot;
			ifstream fin(fileName.c_str(), fstream::binary);
			fin.seekg(0, fin.end);
			m_FileSize = fin.tellg();
			fin.seekg(0, fin.beg);
			fin.read(fileBuffer, m_FileSize);
			fin.close();
			m_SegmentSize = segSize * 1024;
			m_ContentSize = contentsize * 1024;
			m_TotalPacketNum = m_FileSize / m_ContentSize + 1;
			cout << "Finish reading file\n";
			
			std::cout << "Total file size:\t" << m_FileSize << std::endl ;
			std::cout << "m_SegmentSize is:\t" << m_SegmentSize << std::endl ;
			std::cout << "m_TotalSegNum is:\t" << m_FileSize / m_SegmentSize + 1 << std::endl ;
			std::cout << "m_ContentSize is:\t" << m_ContentSize << std::endl ;
			std::cout << "Total packet num is:\t" << m_FileSize / m_ContentSize + 1 << std::endl ;
			timeval start, end;
			gettimeofday(&start, 0);
			for(int id = 0;id < m_TotalPacketNum; id++){
				Name fileName = Name("askfile");
				
				fileName.appendNumber(xxx);
				fileName.appendSegment(1024);
				fileName.appendSegmentOffset(id);
				
				shared_ptr<Data> data =  make_shared<Data>(fileName);
				if(isSign == 1)
					m_keyChain.sign(*data);
				else
					m_keyChain.signWithSha256(*data);
				
				m_store.push_back(data);
				
				if(id == m_TotalPacketNum - 1)
					std::cout <<"Finish signing last datapacket\n";
			}
			
			gettimeofday(&end, 0);
			double totalTime =(1000000.0 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec ) / 1000000;
			std::cout << "Signing cost is:\t" << totalTime <<" s" << std::endl;
		};

		void run(){
 			m_face.setInterestFilter(namePrefix, bind(&NdnServer::onHelloInterest, this, _1, _2), RegisterPrefixSuccessCallback(), bind(&NdnServer::onRegisterFailed, this, _1, _2));

 			m_face.setInterestFilter("askfile", bind(&NdnServer::onFileInterest, this, _1, _2), RegisterPrefixSuccessCallback(), bind(&NdnServer::onRegisterFailed, this, _1, _2));

			std::cout << "start process events\n";
 			m_face.processEvents();
		}
	private:
		Face m_face;
		KeyChain m_keyChain;
		std::vector<shared_ptr<Data>> m_store;
		bool isSign;
		
		void onHelloInterest(const InterestFilter& filter, const Interest& interest){
// 			std::cout <<"Received hello interest\n";
 			std::cout << "\n\n\n----------this is test No. "  <<++count << "------\n"<< std::endl;

			
			Name fileName(interest.getName());
			int *content = new int[3];
			content[0] = m_FileSize;
			content[1] = m_SegmentSize;
			content[2] = m_ContentSize;
			
			
			fileBuffer = new char[m_FileSize];
			
			
			shared_ptr<Data> data = make_shared<Data>();
			data->setName(fileName);
			data->setFreshnessPeriod(time::seconds(10));
			data->setContent(reinterpret_cast<const uint8_t*>(content), 3 * sizeof(int));
			m_keyChain.sign(*data);
// 			m_keyChain.signWithSha256(*data);
			m_face.put(*data);
	
		}
		

		
		void onFileInterest(const InterestFilter& filter, const Interest& interest){
			int currentPacketID = interest.getName().get(-1).toSegmentOffset();
			
			m_face.put(*m_store[currentPacketID]);
			std::cout <<"RECEVE INTerest, send data id:\t" << currentPacketID << std::endl;
			
		}

		void onRegisterFailed(const Name& prefix, const std::string& reason){
			std::cerr <<"ERROR: Failed to register prefix \"" << prefix << "\" in local hub's daemon("<< reason << ")\n";
			m_face.shutdown();
		}
	};
}


int main(int argc, char* argv[]){
	namePrefix = argv[1];
	std::string fileName = "/home/hult/server-file/acg.mkv";
	xxx = atoi(argv[5]);
	ndn::NdnServer mServer(fileName,atoi(argv[2]), atoi(argv[3]), atoi(argv[4]) );

	try{
		mServer.run();
	}catch(const std::exception& e){
		std::cerr <<"ERROR:\t" << e.what() <<std::endl;
	}
	
	return 0;
}



