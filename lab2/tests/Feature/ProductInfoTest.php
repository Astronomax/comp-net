<?php

namespace Tests\Feature;

use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Support\Facades\DB;
use Illuminate\Testing\TestResponse;
use Tests\TestCase;
use Illuminate\Foundation\Testing\WithoutMiddleware;

class ProductInfoTest extends TestCase
{
    use RefreshDatabase, WithoutMiddleware;

    function setupTables(): void
    {
        DB::select("SELECT setval('products_id_seq', 1, false)");

        $query = <<<EOD
        INSERT INTO products(name, description, image_url)
        VALUES
            ('huawei watch 3', 'smart watch', 'huawei_watch_3.jpg')
        EOD;
        DB::insert($query);
    }

    public function makeInfoQuery(int $product_id) : TestResponse
    {
        return $this->withHeader('Accept', 'application/json')
            ->get("/api/product/info/{$product_id}");
    }

    /**
     * @dataProvider DataProviderInfo
     */
    public function testProductInfo(int $product_id, int $status_code, array $expected) : void
    {
        $this->setupTables();
        $response = $this->makeInfoQuery($product_id);
        $response->assertStatus($status_code);
        $response->assertJson($expected);
    }

    public function DataProviderInfo() : array
    {
        return [
            [1, 201, ["name" => "huawei watch 3", 'description' => 'smart watch', "image_url" => "huawei_watch_3.jpg"]], // all is ok
            [3, 404, ['message' => 'Product not found']] // product not exists
        ];
    }
}
