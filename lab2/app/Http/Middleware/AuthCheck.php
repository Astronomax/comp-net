<?php

namespace App\Http\Middleware;

use App\Models\SessionToken;
use Closure;
use Illuminate\Http\Request;

class AuthCheck
{
    public function handle(Request $request, Closure $next)
    {
        $userId = $request->cookie('user_id');
        $sessionToken = $request->cookie('session_token');
        $userIdHeader = config('auth_settings')['userIdHeader'];
        $request->headers->set($userIdHeader, null);

        // Can we add header?
        if ($userId && $sessionToken) {
            if (SessionToken::checkToken($userId, $sessionToken)) {
                $request->headers->set($userIdHeader, $userId);
            }
        }

        return $next($request);
    }
}
