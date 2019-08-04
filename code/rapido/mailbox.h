
#ifndef MAILBOX_H
#define MAILBOX_H

#include <memory.h>

namespace rapido
{
	class Mailbox
	{
		inline static unsigned int MailboxHash(int index) { return index & 15; }
		int _mailBox[16];

	public:
		Mailbox()
		{
			memset(_mailBox, 0xffffffff, sizeof(_mailBox)); // 0xffffffff -> NoHit
		}

		inline bool SkipIntersectionTestWith(int index)
		{
			unsigned int hash = MailboxHash(index);
			bool skip = (_mailBox[hash] == index);
			_mailBox[hash] = index;
			return skip;
		}
	};
}

#endif // MAILBOX_H

