#ifndef COMMENT_H_
#define COMMENT_H_

#include <string>
#include <utility>

class SaveComment
{
public:
	int authorID;
	std::string authorName;
	std::string authorNameFormatted;
	std::string comment;
	SaveComment(int userID, std::string username, std::string usernameFormatted, std::string commentText):
			authorID(userID), authorName(std::move(username)), authorNameFormatted(std::move(usernameFormatted)), comment(std::move(commentText))
	{
	}
	SaveComment(const SaveComment & comment)
	= default;
	SaveComment(const SaveComment * comment):
			authorID(comment->authorID), authorName(comment->authorName), authorNameFormatted(comment->authorNameFormatted), comment(comment->comment)
	{
	}
};


#endif /* COMMENT_H_ */
