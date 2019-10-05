<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console functionality.
 */

use PHPUnit\Framework\TestCase;

class CoreTest extends TestCase {

  /**
   * Make sure running `b` works.
   */
  public function testBInstalledCorrectly() {
    exec('b', $output);
    $this->assertContains('Backdrop Console commands:', $output);
  }

}

