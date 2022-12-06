<?php
/**
 * @file
 * PHPUnit tests for Bee Cache commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Tests the Bee cache commands.
 */
class CacheCommandsTest extends TestCase {

  /**
   * Make sure that the cache-clear command works.
   */
  public function test_cache_clear_command_works() {
    $output_all = shell_exec('bee cache-clear all');
    $this->assertStringContainsString('Cache(s) cleared: All', (string) $output_all);

    $output_menu = shell_exec('bee cache-clear menu');
    $this->assertStringContainsString('Cache(s) cleared: Menu', (string) $output_menu);
  }

}
