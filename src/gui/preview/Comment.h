#pragma once
#include "common/String.h"

class SaveComment
{
public:
	int authorID;
	ByteString authorName;
	ByteString authorNameFormatted;
	String comment;
	SaveComment(int userID, ByteString username, ByteString usernameFormatted, String commentText):
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

