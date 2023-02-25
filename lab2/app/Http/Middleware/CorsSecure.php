<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;

class CorsSecure
{
    /**
     * Handle an incoming request.
     *
     * @param \Illuminate\Http\Request $request
     * @param \Closure(\Illuminate\Http\Request): (\Illuminate\Http\Response|\Illuminate\Http\RedirectResponse) $next
     * @return \Illuminate\Http\Response|\Illuminate\Http\RedirectResponse
     */
    public function handle(Request $request, Closure $next)
    {
        $result = $next($request);
        return $result->withHeaders([
            // now we accept queries from everywhere for testing
            "Access-Control-Allow-Origin" => $request->header('Origin'),
            'Access-Control-Allow-Headers' => 'Origin, X-Requested-With, Content-Type, Accept',
            "Access-Control-Allow-Credentials" => "true"
        ]);
    }
}
