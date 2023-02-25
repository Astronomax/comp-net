<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Support\Facades\DB;

class Product extends Model
{
    use HasFactory;
    protected $table = 'products';
    protected $fillable = [
        'name',
        'description',
        'image_url',
    ];

    public static function getById(string $id): \stdClass|null
    {
        $product = DB::table('products')
            ->select('*')
            ->where('id', '=', $id)
            ->first();
        if($product && $product->deleted_at) {
            return null;
        }
        return $product;
    }
    public static function isProductExists(string $id): bool
    {
        return self::getById($id) !== null;
    }
    public static function createProduct($product): int
    {
        return DB::table('products')->insertGetId($product);
    }
    public static function updateProduct($parameters, $id): bool
    {
        return DB::table('products')->where('id', '=', $id)->update($parameters);
    }
    public static function deleteProduct($id): bool
    {
        return DB::table('products')->where('id', '=', $id)->update(["deleted_at" => DB::raw('CURRENT_TIMESTAMP(0)'),]);
    }
}