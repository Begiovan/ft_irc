// queste funzioni vanno inserite nella classe server per gestire i cambi di mode del channel una volta che il parser riconosce il comando
class Client;
class Channel;


#include <string>
#include <iostream>
// /TOPIC : se non c'è una stringa topic, stampa il topic del chan, se invece gliela passiamo, controlla se ha i permessi e lo cambia

void handleTopic(Channel &channel, std::string &topic){

	if (topic.empty())
		std::cout << channel.getTopic() << std::endl;
	if(channel.canChangeTopic(this))
		channel.setTopic(topic);
	else
		std::cout << _nickName << " don't have permission to change the topic" << std::endl;
}

// KICK

void handleKick(Channel &channel, Client &other){
	if(channel.isOperator(this))
		channel.removeMember(other);
	else
		std::cout << _nickName << " don't have permission to change the topic" << std::endl;
}










