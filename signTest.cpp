#include <iostream>
#include <string>
#include <fstream>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ctime>

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

static uint8_t content[1024*1024];



int main(int argc, char* argv[]){
	using namespace std;
	KeyChain m_keyChain;
	Name dataName("ndn");
	dataName.append("test");
	dataName.appendNumber(1024);
	dataName.appendNumber(8192);
	
	cout << dataName.get(-2).toNumber() <<endl;
	cout << dataName.get(-1).toNumber() << endl;
	
	
	
	
	/*
	for(int i = 0; i < 8* 1024; i++){
		content[i] = (i % 25) + 'a';
	}
	clock_t finish, start;
	
	start = clock();
	for(int i = 0; i < 10000; i++){
		
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(content, 8* 1024);
		m_keyChain.sign(*data);
 		data.reset();
	};
	finish = clock();
	cout << "total time is:\t" << (finish - start) / 1000000 << "s "<<(finish - start) % 1000000 / 1000 << "ms\n";
	 
	start = clock();
	for(int i = 0; i < 10000; i++){
		
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(content, 8* 1024);
		m_keyChain.signWithSha256(*data);
 		data.reset();
	};
	finish = clock();
	cout << "total time is:\t" << (finish - start) / 1000000 << "s "<<(finish - start) % 1000000 / 1000 << "ms\n";
	
	start = clock();
	for(int i = 0; i < 10000; i++){
		
		shared_ptr<Data> data = make_shared<Data>();
		data->setName(dataName);
		data->setFreshnessPeriod(time::seconds(10));
		data->setContent(content, 8* 1024);
 		m_keyChain.signWithSha256(*data);
 		data.reset();
	};
	finish = clock();
	cout << "total time is:\t" << (finish - start) / 1000000 << "s "<<(finish - start) % 1000000 / 1000 << "ms\n";

	*/
	
	return 0;
}
	
