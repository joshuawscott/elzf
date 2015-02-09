%% lzf_test.erl -- unit test for wrapper of the elzf library
%%
%% This file is part of the erlang elzf data compression library.
%% Copyright (C) 2015 Oleksandr Chumachenko
%% All Rights Reserved.
%%
%% The elzf library is free software; you can redistribute it and/or
%% modify it under the terms of the GNU General Public License as
%% published by the Free Software Foundation; either version 2 of
%% the License, or (at your option) any later version.
%%
%% The elzf library is distributed in the hope that it will be useful,
%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%% GNU General Public License for more details.
%% You should have received a copy of the GNU General Public License
%% along with the elzf library; see the file COPYING.
%% If not, write to the Free Software Foundation, Inc.,
%% 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
%%
%% Oleksandr Chumachenko
%% <ledest@gmail.com>
%% https://github.com/Ledest/elzf/

-module(lzf_tests).
-include_lib("eunit/include/eunit.hrl").

lzf_test_() ->
    {timeout, 60, [fun lzf/0]}.

lzf() ->
    {ok, Data} = file:read_file("../README.md"),
    io:fwrite("~p~n", [file:get_cwd()]),
    Compressed1 = lzf:zip(Data),
    Decompressed1 = lzf:unzip(Compressed1, size(Data) * 256),
    Decompressed2 = lzf:unzip(Compressed1, size(Data)),
    Decompressed3 = lzf:unzip(Compressed1),
    {ok, Compressed4} = lzf:compress(Data),
    {ok, Decompressed4} = lzf:decompress(Compressed4),
    ?assertEqual(Data, Decompressed1),
    ?assertEqual(Data, Decompressed2),
    ?assertEqual(Data, Decompressed3),
    ?assertEqual(Data, Decompressed4).
