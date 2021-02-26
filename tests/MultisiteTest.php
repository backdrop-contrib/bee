<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console multisite functionality.
 */

use PHPUnit\Framework\TestCase;

class MultiCoreTest extends TestCase {

  /**
   * Make sure that the `--site` option works.
   *
   * Test running `b` from a multisite installation without the '--site' option,
   * and then with the '--site' option - once using the site URL, and once using
   * the site directory name.
   */
  public function test_site_global_option_works() {
    $output_no_site = shell_exec('b status');
    $this->assertRegExp('/Site type +Multisite/', $output_no_site);
    $this->assertNotRegExp('/Site directory +multisite/', $output_no_site);

    $output_site_dir = shell_exec('b --site=multisite status');
    $this->assertRegExp('/Site type +Multisite/', $output_site_dir);
    $this->assertRegExp('/Site directory +multisite/', $output_site_dir);

    $output_site_url = shell_exec('b --site=b-multi.lndo.site status');
    $this->assertRegExp('/Site type +Multisite/', $output_site_url);
    $this->assertRegExp('/Site directory +multisite/', $output_site_url);
  }

}
