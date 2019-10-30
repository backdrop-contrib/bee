<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console multisite PM commands.
 */

use PHPUnit\Framework\TestCase;

class MultisiteCommandsPMTest extends TestCase {

  /**
   * Check that the PM Download command works for multisites.
   */
  public function testPmDownloadMultisite() {
    $root = shell_exec('pwd');

    // Root directory.
    $output_root = shell_exec('b dl simplify');
    $this->assertStringContainsString('simplify downloaded to ' . $root . '/modules/simplify', $output_root);
    $this->assertTrue(file_exists('modules/simplify/simplify.info'));

    // Site directory.
    $output_site = shell_exec('cd sites/multisite && b dl borg');
    $this->assertStringContainsString('borg downloaded to ' . $root . '/sites/multisite/themes/borg', $output_site);
    $this->assertTrue(file_exists('sites/multisite/themes/borg/borg.info'));

    // Remove downloads for future tests.
    exec('rm -r modules/simplify sites/multisite/themes/borg');
  }

}
