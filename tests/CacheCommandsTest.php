<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Cache commands.
 */

use PHPUnit\Framework\TestCase;

class CacheCommandsTest extends TestCase {

  /**
   * Make sure that the cache-clear command works.
   */
  public function test_cache_clear_command_works() {
    $output_all = shell_exec('b cache-clear all');
    $this->assertStringContainsString('Cache(s) cleared: All', $output_all);

    $output_menu = shell_exec('b cache-clear menu');
    $this->assertStringContainsString('Cache(s) cleared: Menu', $output_menu);
  }

}
