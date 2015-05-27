#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ndn-cxx/face.hpp>
#include <time.h>


const static std::string filename = "/home/netlab/acceptedFile";
int xxx = 0;
void putDataToBuffer(ndn::Data dataPacket, char *buffer){
	const ndn::Block& block = dataPacket.getContent();
	memcpy(buffer, (char*)(reinterpret_cast<const uint8_t*>(block.value())), block.value_size());
}

void addBufferToFile(char *buffer,size_t bufferSize,  std::string filename){
	std::ofstream fout(filename.c_str(), std::fstream::binary);
	
	//problem seem to be here
	fout.write(buffer, bufferSize);
	fout.close();
	
}


char *namePrefix  = new char[256];
namespace ndn {
	namespace examples {
		class NdnClient: noncopyable{
		public:
			void run(){
				timeval start, end;
				gettimeofday(&start, 0);
				expressHelloInterest();
 				expressFileInterest();
				gettimeofday(&end, 0);
				double totalTime =(1000000.0 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec ) / 1000000;
				std::cout << "----------------RESULT-----------------\n";
				std::cout <<"Interest per time:\t" << m_SegmentSize / m_ContentSize << std::endl;
				std::cout <<"Content size:\t\t" << m_ContentSize<<  std::endl;
				std::cout <<"Throughput:\t\t" << (double)(m_FileSize) / (1024.0 * 1024.0) /totalTime <<" MBps" <<std::endl;
				std::cout << std::endl;
				addBufferToFile(fileBuffer, m_FileSize, filename);	
			}	
			
			void expressHelloInterest(){
				Name tempName(namePrefix);
				tempName.appendVersion();
				Interest helloInterest(tempName);
				helloInterest.setInterestLifetime(time::seconds(10));
				helloInterest.setMustBeFresh(true);

				m_face.expressInterest(helloInterest, bind(&NdnClient::onHelloData, this, _1, _2), bind(&NdnClient::onHelloTimeout, this, _1));
				m_face.processEvents();
				
			}
			
			void expressFileInterest(){
				int totalDataPacketNum = m_FileSize / m_ContentSize + 1;
				for(m_DataPacketID = 0; m_DataPacketID < totalDataPacketNum; m_DataPacketID++){
					int currentSegmentID = m_DataPacketID * m_ContentSize / m_SegmentSize;
					Name fileName = Name("askfile");
					fileName.appendNumber(xxx);
					fileName.appendSegment(1024);
					fileName.appendSegmentOffset(m_DataPacketID);
					
					Interest RequireFileInterest(fileName);
					RequireFileInterest.setInterestLifetime(time::milliseconds(10000));
 					RequireFileInterest.setMustBeFresh(false);
					m_face.expressInterest(RequireFileInterest, bind(&NdnClient::onFileData, this, _1, _2), bind(&NdnClient::onFileTimeout, this, _1) );
					if((1+m_DataPacketID) * m_ContentSize % m_SegmentSize == 0 || m_DataPacketID == totalDataPacketNum -1)
						m_face.processEvents();
				}
			}
		
		public:
			size_t m_TotalSegNum;
			size_t m_ContentSize;
			size_t m_SegmentSize;
			size_t m_FileSize;
			size_t m_DataPacketID;
			char* fileBuffer;
		private:
			Face m_face;
		private:
			void onHelloData(const Interest& interest, const Data& data){
				int* content = new int[3];
				content = (int*)(data.getContent().value());
				m_FileSize = *content;
				m_SegmentSize = *(1 + content);
				m_ContentSize = *(2 + content);
				
				
				std::cout << "Recevd file size:\t" << m_FileSize << std::endl ;
				std::cout << "m_SegmentSize is:\t" << m_SegmentSize << std::endl ;
				std::cout << "m_TotalSegNum is:\t" << m_FileSize / m_SegmentSize + 1 << std::endl ;
				std::cout << "m_ContentSize is:\t" << m_ContentSize << std::endl ;
				std::cout << "Total packet num is:\t" << m_FileSize / m_ContentSize + 1 << std::endl ;
				
				fileBuffer = new char[m_FileSize];
			}
			
			void onFileData(const Interest& interest, const Data& data){
				//data packet has no version
				int dataPacketID = interest.getName().get(-1).toNumber();
				if(dataPacketID == 0 || dataPacketID == m_FileSize / m_ContentSize - 1)
	 				std::cout <<"accepted data packet id:\t" << dataPacketID << std::endl;
					 
				putDataToBuffer(data, fileBuffer + dataPacketID * m_ContentSize);
// 				std::string filename = "/home/hult/ndn-cxx/result.txt";
			}
			
			void onHelloTimeout(const Interest& interest){
				std::cout << "Hello interest Timeout, retransmitting~~\n";
				m_face.expressInterest(interest, bind(&NdnClient::onHelloData, this, _1, _2), bind(&NdnClient::onHelloTimeout, this, _1));
			}
				
			void onFileTimeout(const Interest& interest){
				std::cout << "File interest Timeout, ID:\t" << interest.getName().get(-1).toNumber() << std::endl;
//  				m_face.expressInterest(interest, bind(&NdnClient::onFileData, this, _1, _2), bind(&NdnClient::onFileTimeout, this, _1));
			}
		};
	}
}


int main(int argc, char* argv[]){
	namePrefix = argv[1];
	xxx = atoi(argv[2]);
	ndn::examples::NdnClient mClient;
	try{
		mClient.run();
	}catch(const std::exception& e){
		std::cerr <<"ERROR:\t" << e.what() <<std::endl;
	}
	
	return 0;
}

