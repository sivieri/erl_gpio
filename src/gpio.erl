-module(gpio).
-export([init/0, setup/0, pin_mode/2, digital_write/2, digital_read/1, analog_write/2]).
-on_load(init/0).

-define(APPNAME, erl_gpio).

init() ->
    case code:priv_dir(?APPNAME) of
        {error, _} -> 
            error_logger:format("~w priv dir not found~n", [?APPNAME]),
            exit(error);
        PrivDir ->
            erlang:load_nif(filename:join([PrivDir, "gpio_drv"]), 0)
    end.

setup() ->
    erlang:nif_error(nif_not_loaded).

pin_mode(_Pin, _Mode) ->
    erlang:nif_error(nif_not_loaded).

digital_write(_Pin, _Value) ->
    erlang:nif_error(nif_not_loaded).

digital_read(_Pin) ->
    erlang:nif_error(nif_not_loaded).

analog_write(_Pin, _Value) ->
    erlang:nif_error(nif_not_loaded).
