<?php

return [
    'userIdHeader' => 'X-user-id',

    // should be <= 255
    "tokenLength" => 128,

    // for 3-rd party queries
    "sameSite" => "lax",

    "lifetime" => 5
];