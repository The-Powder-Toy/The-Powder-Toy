#include "Platform.h"

namespace Platform
{
void OpenURI(ByteString uri)
{
	fprintf(stderr, "cannot open URI: not implemented\n");
}

bool CanUpdate()
{
	return false;
}

void SetupCrt()
{
}
}
