<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console multisite functionality.
 */

use PHPUnit\Framework\TestCase;

class MultisiteTest extends TestCase {

  /**
   * Make sure that the `--url` option works.
   *
   * Test running `b` from a multisite installation without the '--url' option,
   * and then with the '--url' option.
   */
  public function testUrlOptionWorks() {
    $output_no_url = shell_exec('b st');
    $this->assertStringContainsString('BackdropCMS is not installed yet', $output_no_url);
    $this->assertStringContainsString('Backdrop CMS Installation detected', $output_no_url);

    $output_url = shell_exec('b --url=127.0.0.1 st');
    $this->assertStringContainsString('Backdrop CMS Installation detected', $output_url);
    $this->assertStringContainsString('Backdrop site', $output_url);
    $this->assertStringContainsString('Database username', $output_url);
  }

}
