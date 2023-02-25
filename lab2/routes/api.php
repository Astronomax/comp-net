<?php

use App\Http\Controllers\NomenclatureController;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\ProductController;
use App\Http\Controllers\RetailerController;
use App\Http\Controllers\BrandController;
use App\Http\Controllers\UserController;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| is assigned the "api" middleware group. Enjoy building your API!
|
*/

Route::controller(ProductController::class)->group(function () {
    Route::post('/product', 'store');
    Route::get('/product/info/{productId}', 'show');
    Route::patch('/product/{productId}', 'update');
    Route::delete('/product/{productId}', 'delete');
});