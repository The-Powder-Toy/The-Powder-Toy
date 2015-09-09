#ifndef USERINFO_H_
#define USERINFO_H_

#include <string>

class UserInfo
{
public:
	int ID;
	int age;
	std::string username;
	std::string biography;
	std::string location;
	std::string website;

	int saveCount;
	float averageScore;
	int highestScore;

	int topicCount;
	int topicReplies;
	int reputation;

	UserInfo(int id, int age, std::string username, std::string biography, std::string location, std::string website, int saveCount, float averageScore, int highestScore, int topicCount, int topicReplies, int reputation):
		ID(id),
		age(age),
		username(username),
		biography(biography),
		location(location),
		website(website),
		saveCount(saveCount),
		averageScore(averageScore),
		highestScore(highestScore),
		topicCount(topicCount),
		topicReplies(topicReplies),
		reputation(reputation)
	{ }
	UserInfo() {}
};


#endif /* USER_H_ */
