<?php

namespace App\Http\Controllers;

use Illuminate\Foundation\Auth\Access\AuthorizesRequests;
use Illuminate\Foundation\Bus\DispatchesJobs;
use Illuminate\Foundation\Validation\ValidatesRequests;
use Illuminate\Http\JsonResponse;
use Illuminate\Routing\Controller as BaseController;

class ApiController extends BaseController
{
    use AuthorizesRequests, DispatchesJobs, ValidatesRequests;

    static function createSuccessResponse($jsonData, $statusCode): JsonResponse
    {
        return response()->json($jsonData, $statusCode);
    }

    static function createErrorResponse($msg, $statusCode): JsonResponse
    {
        return response()->json(['message' => $msg], $statusCode);
    }
}
