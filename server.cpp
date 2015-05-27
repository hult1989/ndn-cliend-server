#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <boost/utility.hpp>
#include <boost/asio.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

using std::string;

const std::string FileName("/home/hult/acg.mkv");
const int _BYTE = 1024;
const int MAX_DA = 1*_BYTE;
const int MAX_FILE = 64*_BYTE*_BYTE;

char DataT[MAX_DA];
char DataA[MAX_FILE];
std::stringstream payloadStream;
std::string Str;

namespace Producer{
	
	class NDNProducer : boost::noncopyable{
		public:
			explicit NDNProducer(char* Name)
				: m_Name(Name)
				, m_Count(0)
			{
				m_inF = fopen(FileName.c_str(),"rb");
				while(fread(&DataA[m_Count],1,1,m_inF)!=0) m_Count++;
				fclose(m_inF);
				m_Total = (m_Count-1)/MAX_DA + 1;
				std::cout<<"Read finish Total Bytes:"<<m_Count<<std::endl;
				std::cout<<"Total Segment:"<<m_Total<<std::endl;
			}

			ndn::shared_ptr<ndn::Data> createdataPacket(const ndn::Interest& interest,size_t Segnum){
				int Size = GetDataSize(Segnum);
				ndn::shared_ptr<ndn::Data> dataPacket = ndn::make_shared<ndn::Data>(interest.getName());
				
				dataPacket->setContent(reinterpret_cast<const uint8_t*>(&DataA[Segnum*MAX_DA]),Size);
				dataPacket->setFreshnessPeriod(ndn::time::milliseconds(10000));

				return dataPacket;
			}
			ndn::shared_ptr<ndn::Data> createdataPacket2(const ndn::Interest& interest,size_t ToT,size_t Seg1,size_t Seg2){
				int Size = GetDataSize(ToT,Seg1,Seg2);	
				ndn::shared_ptr<ndn::Data> dataPacket = ndn::make_shared<ndn::Data>(interest.getName());

				dataPacket->setContent(reinterpret_cast<const uint8_t*>(&DataA[Seg1*MAX_DA+Seg2*_BYTE]),Size);
				dataPacket->setFreshnessPeriod(ndn::time::milliseconds(10000));

				return dataPacket;
			}

			void onInterest(const ndn::Name& name,
							const ndn::Interest& interest){
				ndn::shared_ptr<ndn::Data> dataPacket;
				if(interest.getName().get(-1).toUri()=="Info"){
					std::stringstream TempStream;
					TempStream << m_Total;
					std::string TempStr = TempStream.str(); 
					dataPacket = ndn::make_shared<ndn::Data>(interest.getName());
					dataPacket->setContent(reinterpret_cast<const uint8_t*>(TempStr.c_str()),sizeof(TempStr.length()));
				}else
				if(interest.getName().get(-1).toUri()=="Info2"){
					size_t Seg = interest.getName().get(-2).toNumber();
					size_t ToT = Count_ToT(Seg);
					std::stringstream TempStream;
					TempStream << ToT;
					std::string TempStr = TempStream.str();
					dataPacket = ndn::make_shared<ndn::Data>(interest.getName());
					dataPacket->setContent(reinterpret_cast<const uint8_t*>(TempStr.c_str()),sizeof(TempStr.length()));
				}else
				if(interest.getName().get(-1).toUri()=="Packet1"){
					size_t Segnum = static_cast<size_t>(interest.getName().get(-2).toSegment());
					dataPacket = createdataPacket(interest,Segnum);
					//return;
				}else
				if(interest.getName().get(-1).toUri()=="Packet2"){
					size_t ToT = static_cast<size_t>(interest.getName().get(-4).toNumber());
					size_t Seg1 = static_cast<size_t>(interest.getName().get(-3).toNumber());
					size_t Seg2 = static_cast<size_t>(interest.getName().get(-2).toSegment());
					dataPacket = createdataPacket2(interest,ToT,Seg1,Seg2);
				}
				m_KeyChain.sign(*dataPacket);
				m_Face.put(*dataPacket);
			}
			void onRegisterFailed(const ndn::Name& prefix,
									const std::string& reason)
			{
			}
			void run(){
				m_Face.setInterestFilter(m_Name,
										ndn::bind(&NDNProducer::onInterest,this,_1,_2),
										ndn::RegisterPrefixSuccessCallback(),
										ndn::bind(&NDNProducer::onRegisterFailed,this,_1,_2));
				m_Face.processEvents();
			}

			int GetDataSize(int Seg){
				if(Seg + 1 < m_Total)return MAX_DA;
				return m_Count % MAX_DA;
			}
			int GetDataSize(int ToT,int Seg1,int Seg2){
				if(Seg1 + 1 < m_Total)return _BYTE;
				if(Seg2 + 1 < ToT)return _BYTE;
				return m_Count % _BYTE;
			}
			int Count_ToT(int Seg){
				if(Seg + 1 < m_Total)return MAX_DA / _BYTE;
				int Count = m_Count % MAX_DA;
				return (Count - 1)/_BYTE + 1;
			}

		private:
			ndn::KeyChain m_KeyChain;
			std::string m_Name;
			size_t m_TotalSeg;
			size_t m_Count;
			size_t m_Total;
			ndn::Face m_Face;
			FILE *m_inF;
	};
}

 String filename = "/hult";

int main(int argc,char* argv[]){
	
	
	
	Producer::NDNProducer Producer(filename);
	Producer.run();
	return 0;
}
