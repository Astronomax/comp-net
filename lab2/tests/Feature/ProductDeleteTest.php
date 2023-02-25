<?php

namespace Tests\Feature;

use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Support\Facades\DB;
use Illuminate\Testing\TestResponse;
use Tests\TestCase;
use Illuminate\Foundation\Testing\WithoutMiddleware;

class ProductDeleteTest extends TestCase
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

    public function makeDeleteQuery(int $product_id) : TestResponse
    {
        return $this->withHeader('Accept', 'application/json')
            ->delete("/api/product/{$product_id}");
    }

    /**
     * @dataProvider DataProviderDelete
     */
    public function testProductDelete(int $product_id, int $status_code, int $expected) : void
    {
        $this->setupTables();
        $response = $this->makeDeleteQuery($product_id);
        $response->assertStatus($status_code);
        $product = DB::table('products')
            ->select('deleted_at')
            ->where('id', '=', $product_id)
            ->first();
        switch ($expected) {
            case 0:
                $this->assertNotNull($product->deleted_at);
                break;
            case 1:
                $this->assertNull($product);
                break;
        }
    }

    public function DataProviderDelete() : array
    {
        return [
            [1, 201, 0], // product has been removed
            [3, 404, 1], // there are no such product
        ];
    }
}
