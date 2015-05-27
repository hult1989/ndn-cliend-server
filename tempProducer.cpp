#include <iostream>
#include <string>
#include <fstream>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

using namespace ndn;

class Producer: noncopyable {
private:
	ndn::Face m_face;
	ndn::KeyChain m_keyChain;
	
	void onInterest(const InterestFilter& filter, const Interest& interest){
		std::cout << "<< I: " << interest << std::endl;
		Name dataName(interest.getName());
		dataName.append("-data-").appendVersion();
		
		static const std::string content = "this is the first data package i generated! ";
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
		m_keyChain.sign(*data);
		std::cout << ">> D: " << *data << std::endl;
		m_face.put(*data);
		
	}
public:
	
	shared_ptr<Data>  createDatePacket(Interest& interest){
		Name dataName(interest.getName());
		dataName.append("-data-").appendVersion();
		std::ifstream fin;
		fin.open("/home/hult/acg.mkv");
		fin.seekg(0, fin.end);
		size_t length = fin.tellg();
		fin.seekg(0, fin.beg);
	
		uint8_t *buffer = new uint8_t[length];
		fin.read((char*)buffer, length);
		
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(buffer, length);
		m_keyChain.sign(*data);
		std::cout << "\n\n======================\n";
		std::cout << ">> D: " << *data << std::endl;
		std::cout << "\n\n======================\n";
		return data;
	}
/*
	
		shared_ptr<Data>  createDatePacket(Interest& interest){
		Name dataName(interest.getName());
		dataName.append("-data-").appendVersion();
		
		static const std::string content = "this is the first data package i generated! ";
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
		m_keyChain.sign(*data);
		std::cout << "\n\n======================\n";
		std::cout << ">> D: " << *data << std::endl;
		std::cout << "\n\n======================\n";
		return data;
	}

	*/
};

void PutDataToFile(ndn::Data data){
	const ndn::Block& block = data.getContent();
	size_t fileSize = block.value_size();
	std::cout << fileSize << std::endl;
	std::ofstream fout;
	fout.open("/home/hult/dataPacketFile");
 	fout.write((char*)(reinterpret_cast<const uint8_t*>(block.value())), block.value_size());
	fout.close();

}




int main(){
	using namespace std;
    String str = "/firstInterest/";
	ndn::Name interestName("/firstInterest/");
 	ndn::Interest interest0(interestName);
 	interest0.setInterestLifetime(time::milliseconds(10000));
 	interest0.setMustBeFresh(false);
 	cout << "Interest is: " << interest0 << endl;
	
	Producer myProducer;
	shared_ptr<Data> dataPacket = myProducer.createDatePacket(interest0);

 	PutDataToFile(*dataPacket);
	
	return 0;
}