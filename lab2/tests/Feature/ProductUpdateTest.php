<?php

namespace Tests\Feature;

use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Support\Facades\DB;
use Illuminate\Testing\TestResponse;
use Tests\TestCase;
use Illuminate\Foundation\Testing\WithoutMiddleware;

class ProductUpdateTest extends TestCase
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

    public function makeUpdateQuery(array $product, int $product_id) : TestResponse
    {
        return $this->withHeader('Accept', 'application/json')
            ->patch("/api/product/{$product_id}", $product);
    }

    /**
     * @dataProvider DataProviderUpdate
     */
    public function testProductUpdate(int $product_id, int $status_code, array $product, array $expected) : void
    {
        $this->setupTables();
        $response = $this->makeUpdateQuery($product, $product_id);
        $response->assertStatus($status_code);
        $product = DB::table('products')
            ->select('name', 'description', 'image_url')
            ->where('id', '=', $product_id)
            ->first();
        $this->assertJsonStringEqualsJsonString(
            json_encode((array) $product), 
            json_encode((array) $expected));
    }

    public function DataProviderUpdate() : array
    {
        return [
            [1, 201, ["name" => "huawei watch 3", 'description' => 'smart watch', "image_url" => "huawei_watch_3.jpg"], ["name" => "huawei watch 3", 'description' => 'smart watch', "image_url" => "huawei_watch_3.jpg"]], // all is ok
        ];
    }

    public function testEmptyUpdate() : void {
        $this->setupTables();
        $response = $this->makeUpdateQuery([], 1);
        $response->assertStatus(500);
        DB::rollBack();
        $product = DB::table('products')
            ->select('name', 'description', 'image_url')
            ->where('id', '=', 1)
            ->first();
        $this->assertNull($product);
    }

    public function testDoesNotExists() : void {
        $response = $this->makeUpdateQuery([], 1);
        $response->assertStatus(404);
        $response->assertJson(['message' => 'Product not found']);
    }
}
