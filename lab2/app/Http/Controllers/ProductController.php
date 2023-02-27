<?php

namespace App\Http\Controllers;

use App\Models\Product;
use App\Models\Retailer;
use Illuminate\Foundation\Bus\DispatchesJobs;
use Illuminate\Foundation\Validation\ValidatesRequests;
use Illuminate\Http\Request;
use Illuminate\Http\JsonResponse;
use Illuminate\Support\Str;

class ProductController extends ApiController
{
    use DispatchesJobs, ValidatesRequests;

    function store(Request $request): JsonResponse
    {
        $product = $request->validate([
            'name' => 'required',
            'description' => 'required',
        ]);
        $fileName = Str::random(10).'.jpg';
        $request->file('image')->move(public_path('/'), $fileName);
        $photoPath = url('/'.$fileName);
        $product['image_url'] = $photoPath;
        $product_id = Product::createProduct($product);
        return $this->createSuccessResponse(['product_id' => $product_id], 201);
    }
    function show(Request $request, int $productId): JsonResponse
    {
        $product = Product::getById($productId);
        return $product ? $this->createSuccessResponse($product, 201) :
            $this->createErrorResponse('Product not found', 404);
    }
    function update(Request $request, int $productId): JsonResponse
    {
        $parameters = $request->validate([
            'name' => '',
            'description' => '',
            'image_url' => '',
        ]);
        $product = Product::getById($productId);
        if(!$product) {
            return $this->createErrorResponse('Product not found', 404);
        }
        Product::updateProduct($parameters, $productId);
        return $this->createSuccessResponse('OK', 201);
    }
    function delete(Request $request, int $productId): JsonResponse
    {
        $product = Product::getById($productId);
        if(!$product) {
            return $this->createErrorResponse('Product not found', 404);
        }
        Product::deleteProduct($productId);
        return $this->createSuccessResponse('OK', 201);
    }
}