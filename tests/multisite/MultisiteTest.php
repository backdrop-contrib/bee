<?php
/**
 * @file
 * PHPUnit tests for core Bee multisite functionality.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's global multisite option.
 */
class MultisiteTest extends TestCase {

  /**
   * Make sure that the `--site` option works.
   *
   * Test running `bee` from a multisite installation without the '--site'
   * option, and then with the '--site' option - once using the site directory
   * name, and once using the site URL.
   */
  public function test_site_global_option_works() {
    global $bee_test_multisite_multi_2_domain;
    $output_no_site = shell_exec('bee status');
    $this->assertMatchesRegularExpression('/Site type +Multisite/', (string) $output_no_site);
    $this->assertDoesNotMatchRegularExpression('/Site directory +multisite/', (string) $output_no_site);

    $output_site_dir = shell_exec('bee --site=multi_one status');
    $this->assertMatchesRegularExpression('/Site type +Multisite/', (string) $output_site_dir);
    $this->assertMatchesRegularExpression('/Site directory +multi_one/', (string) $output_site_dir);

    $output_site_url = shell_exec("bee --site=$bee_test_multisite_multi_2_domain status");
    $this->assertMatchesRegularExpression('/Site type +Multisite/', (string) $output_site_url);
    $this->assertMatchesRegularExpression('/Site directory +multi_two/', (string) $output_site_url);
  }

}
