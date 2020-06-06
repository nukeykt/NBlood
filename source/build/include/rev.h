#pragma once

#ifndef rev_h__
#define rev_h__

#if defined REV
# define REV__(x) #x
# define REV_(x) REV__(x)
# define REVSTR REV_(REV)
#else
# define REVSTR "r(?)"
# define EDUKE32_UNKNOWN_REVISION
#endif
#endif // rev_h__
