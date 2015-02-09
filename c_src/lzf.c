/* lzf.c -- wrapper of the LZF algorithm

   This file is part of the erlang elzf data compression library.

   Copyright (C) 2015 Oleksandr Chumachenko
   All Rights Reserved.

   The elzf library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The elzf library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the elzf library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Oleksandr Chumachenko
   <ledest@gmail.com>
   https://github.com/Ledest/elzf/
 */

#include <erl_nif.h>
#include <lzf.h>
#include <stdint.h>

static inline ERL_NIF_TERM make_ok(ErlNifEnv *env, ERL_NIF_TERM msg)
{
	return enif_make_tuple2(env, enif_make_atom(env, "ok"), msg);
}

static inline ERL_NIF_TERM make_error(ErlNifEnv *env, const char *msg)
{
	return enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_atom(env, msg));
}

static ERL_NIF_TERM lzf_zip(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
	ErlNifBinary i, o;
	unsigned os;
	const char *s = "insufficient_memory";

	if (!enif_inspect_binary(env, argv[0], &i) && !enif_inspect_iolist_as_binary(env, argv[0], &i))
		return enif_make_badarg(env);

	if (!enif_alloc_binary(i.size + (i.size + 24) / 25, &o))
		return make_error(env, s);

	os = lzf_compress((const void*)i.data, (unsigned)i.size, o.data, o.size);
	if (os) {
		if (os == o.size || enif_realloc_binary(&o, os))
			return enif_make_binary(env, &o);
		s = "unknown";
	}
	return make_error(env, s);
}

static ERL_NIF_TERM lzf_unzip(ErlNifEnv* env, ErlNifBinary *i, unsigned ms)
{
	ErlNifBinary o;
	unsigned os;

	if (!enif_alloc_binary(ms, &o))
		return make_error(env, "insufficient_memory");

	os = lzf_decompress((const void*)i->data, i->size, o.data, o.size);
	return !os
	       ? make_error(env, "unknown")
	       : (os == o.size || enif_realloc_binary(&o, os))
	         ? enif_make_binary(env, &o)
	         : make_error(env, "insufficient_memory");
}

static ERL_NIF_TERM lzf_unzip_2(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
	ErlNifBinary i;
	unsigned ms;

	return (enif_get_uint(env, argv[1], &ms) && enif_inspect_iolist_as_binary(env, argv[0], &i))
	       ? lzf_unzip(env, &i, ms)
	       : enif_make_badarg(env);
}

static ERL_NIF_TERM lzf_unzip_1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
	ErlNifBinary i;

	return (enif_inspect_binary(env, argv[0], &i) || enif_inspect_iolist_as_binary(env, argv[0], &i))
	       ? lzf_unzip(env, &i, i.size * 256)
	       : enif_make_badarg(env);
}

static ERL_NIF_TERM lzf_compress_1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
	ErlNifBinary i, o;
	unsigned os;
	const char *s = "insufficient_memory";

	if (!enif_inspect_binary(env, argv[0], &i) || !enif_inspect_iolist_as_binary(env, argv[0], &i))
		return enif_make_badarg(env);

	if (!enif_alloc_binary(i.size + (i.size + 24) / 25 + sizeof(uint32_t), &o))
		return make_error(env, s);

	*(uint32_t*)o.data = (uint32_t)i.size;
	os = lzf_compress((const void*)i.data, i.size, o.data + sizeof(uint32_t), o.size - sizeof(uint32_t));
	if (os) {
		if (os == o.size || enif_realloc_binary(&o, os + sizeof(uint32_t)))
			return make_ok(env, enif_make_binary(env, &o));
		s = "unknown";
	}
	return make_error(env, s);
}

static ERL_NIF_TERM lzf_decompress_1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
	ErlNifBinary i, o;
	unsigned os;

	if (!enif_inspect_binary(env, argv[0], &i) || !enif_inspect_iolist_as_binary(env, argv[0], &i))
		return enif_make_badarg(env);

	if (!enif_alloc_binary(*(uint32_t*)i.data, &o))
		return make_error(env, "insufficient_memory");

	os = lzf_decompress((const void*)i.data + sizeof(uint32_t), i.size - sizeof(uint32_t), o.data, o.size);
	return os
	       ? make_ok(env, enif_make_binary(env, &o))
	       : make_error(env, "unknown");
}

static int on_load(ErlNifEnv *env, void **priv, ERL_NIF_TERM info)
{
	return 0;
}

static int on_reload(ErlNifEnv *env, void**priv, ERL_NIF_TERM info)
{
	return 0;
}

static int on_upgrade(ErlNifEnv *env, void **priv, void** old_priv, ERL_NIF_TERM info)
{
	return 0;
}


static ErlNifFunc nif_functions[] = {
	{"zip", 1, lzf_zip},
	{"unzip", 2, lzf_unzip_2},
	{"unzip", 1, lzf_unzip_1},
	{"compress", 1, lzf_compress_1},
	{"uncompress", 1, lzf_decompress_1},
	{"decompress", 1, lzf_decompress_1}
};

ERL_NIF_INIT(lzf, nif_functions, &on_load, &on_reload, &on_upgrade, NULL);
