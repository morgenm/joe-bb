/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

static ZHENTRY *freentry = NULL;

/* Compute hash value of string */

#define hnext(accu, c) (((accu) << 4) + ((accu) >> 28) + (c))

ptrdiff_t Zhash(const int *s)
{
	ptrdiff_t accu = 0;

	while (*s) {
		accu = hnext(accu, *s++);
	}
	return accu;
}

/* Create hash table */

ZHASH *Zhtmk(ptrdiff_t len)
{
	ZHASH *t = (ZHASH *) joe_malloc(SIZEOF(ZHASH));
	t->nentries = 0;
	t->len = len;
	t->tab = (ZHENTRY **) joe_calloc(SIZEOF(ZHENTRY *), len);
	return t;
}

/* Delete hash table.  Only the hash table is deleted, not the names and values */

void Zhtrm(ZHASH *ht)
{
	ptrdiff_t x;
	for (x = 0; x != ht->len; ++x) {
		ZHENTRY *p, *n;
		for (p = ht->tab[x]; p; p = n) {
			n = p->next;
			p->next = freentry;
			freentry = p;
		}
	}
	joe_free(ht->tab);
	joe_free(ht);
}

/* Expand hash table */

static void Zhtexpand(ZHASH *h)
{
	ptrdiff_t x;
	/* Allocate new table */
	ptrdiff_t new_size = h->len * 2;
	ZHENTRY **new_table = (ZHENTRY **)joe_calloc(new_size, SIZEOF(ZHENTRY *));
	/* Copy entries from old table to new */
	for (x = 0; x != h->len; ++x) {
		ZHENTRY *e;
		while ((e = h->tab[x])) {
			h->tab[x] = e->next;
			e->next = new_table[e->hash_val & (new_size - 1)];
			new_table[e->hash_val & (new_size - 1)] = e;
		}
	}
	/* Replace old table with new */
	free(h->tab);
	h->tab = new_table;
	h->len = new_size;
}

/* Bind a value to a name.  This does not check for duplicate entries.  The
 * name and value are not duplicated: it's up to you to keep them around for
 * the life of the hash table. */

void *Zhtadd(ZHASH *ht, const int *name, void *val)
{
	ptrdiff_t hval = Zhash(name);
	ptrdiff_t idx = hval & (ht->len - 1);
	ZHENTRY *entry;
	ptrdiff_t x;

	if (!freentry) {
		entry = (ZHENTRY *) joe_malloc(SIZEOF(ZHENTRY) * 64);
		for (x = 0; x != 64; ++x) {
			entry[x].next = freentry;
			freentry = entry + x;
		}
	}
	entry = freentry;
	freentry = entry->next;
	entry->next = ht->tab[idx];
	ht->tab[idx] = entry;
	entry->name = name;
	entry->val = val;
	entry->hash_val = hval;
	if (++ht->nentries == (ht->len >> 1) + (ht->len >> 2))
		Zhtexpand(ht);
	return val;
}

/* Return value associated with name or NULL if there is none */

void *Zhtfind(ZHASH *ht, const int *name)
{
	ZHENTRY *e;

	for (e = ht->tab[Zhash(name) & (ht->len - 1)]; e; e = e->next) {
		if (!Zcmp(e->name, name)) {
			return e->val;
		}
	}
	return NULL;
}