/*
 * Comment.h
 *
 *  Created on: Feb 11, 2012
 *      Author: Simon
 */

#ifndef COMMENT_H_
#define COMMENT_H_

class Comment
{
public:
	int authorID;
	std::string authorName;
	std::string comment;
	Comment(int userID, std::string username, std::string commentText):
		authorID(userID), authorName(username), comment(commentText)
	{
	}
};


#endif /* COMMENT_H_ */
