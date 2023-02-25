<?php

namespace Tests\Feature;

use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Support\Facades\DB;
use Illuminate\Testing\TestResponse;
use Tests\TestCase;
use Illuminate\Foundation\Testing\WithoutMiddleware;

class ProductCreateTest extends TestCase
{
    use RefreshDatabase, WithoutMiddleware;

    public function makeCreateQuery(array $product) : TestResponse
    {
        return $this->withHeader('Accept', 'application/json')
            ->post("/api/product/", $product);
    }

    /**
     * @dataProvider DataProviderCreate
     */
    public function testProductCreate(array $product, int $status_code, array $expected) : void
    {
        $response = $this->makeCreateQuery($product);
        $response->assertStatus($status_code);
        $product_id = $response["product_id"];
        $product = DB::table('products')
            ->select('name', 'description', 'image_url')
            ->where('id', '=', $product_id)
            ->first();
        $this->assertJsonStringEqualsJsonString(
            json_encode((array) $product), 
            json_encode((array) $expected));
    }

    public function DataProviderCreate() : array
    {
        return [
            [["name" => "huawei watch 3", 'description' => 'smart watch', "image_url" => "huawei_watch_3.jpg"], 201, ["name" => "huawei watch 3", 'description' => 'smart watch', "image_url" => "huawei_watch_3.jpg"]], // all is ok
        ];
    }

    public function testRequiredFieldNotAssigned() : void {
        $product = [];
        $response = $this->makeCreateQuery($product);
        $response->assertStatus(422);
    }
}
