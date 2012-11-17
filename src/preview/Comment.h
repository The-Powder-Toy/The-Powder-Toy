/*
 * Comment.h
 *
 *  Created on: Feb 11, 2012
 *      Author: Simon
 */

#ifndef COMMENT_H_
#define COMMENT_H_

#include <string>

class SaveComment
{
public:
	int authorID;
	std::string authorName;
	std::string comment;
	SaveComment(int userID, std::string username, std::string commentText):
			authorID(userID), authorName(username), comment(commentText)
	{
	}
	SaveComment(const SaveComment & comment):
			authorID(comment.authorID), authorName(comment.authorName), comment(comment.comment)
	{
	}
	SaveComment(const SaveComment * comment):
			authorID(comment->authorID), authorName(comment->authorName), comment(comment->comment)
	{
	}
};


#endif /* COMMENT_H_ */
