#ifndef COMMENT_H_
#define COMMENT_H_

#include <string>

class SaveComment
{
public:
	int authorID;
	std::string authorName;
	std::string authorNameFormatted;
	std::string comment;
	SaveComment(int userID, std::string username, std::string usernameFormatted, std::string commentText):
			authorID(userID), authorName(username), authorNameFormatted(usernameFormatted), comment(commentText)
	{
	}
	SaveComment(const SaveComment & comment):
			authorID(comment.authorID), authorName(comment.authorName), authorNameFormatted(comment.authorNameFormatted), comment(comment.comment)
	{
	}
	SaveComment(const SaveComment * comment):
			authorID(comment->authorID), authorName(comment->authorName), authorNameFormatted(comment->authorNameFormatted), comment(comment->comment)
	{
	}
};


#endif /* COMMENT_H_ */
