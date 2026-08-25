#!/usr/bin/env python3
"""
Test for memory leak fix in picologging issue #223.
"""
import unittest
import sys
import tracemalloc
import picologging


class TestMemoryLeak(unittest.TestCase):
    
    def setUp(self):
        # Configure picologging
        picologging.basicConfig(
            level=picologging.INFO, 
            stream=sys.stdout, 
            format="%(asctime)s - %(levelname)s - %(message)s"
        )
        
    def test_no_memory_leak_basic_logging(self):
        """Test that basic logging doesn't leak memory."""
        tracemalloc.start()
        
        logger = picologging.getLogger("test_basic")
        
        # Take initial snapshot
        snapshot1 = tracemalloc.take_snapshot()
        
        # Log many messages
        for i in range(1000):
            logger.info("test message %d", i)
        
        # Take final snapshot
        snapshot2 = tracemalloc.take_snapshot()
        
        # Check memory growth
        top_stats = snapshot2.compare_to(snapshot1, 'lineno')
        
        # The memory growth should be minimal (less than 1MB for 1000 messages)
        current, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        
        # This is a reasonable threshold - logging 1000 messages shouldn't use more than 5MB
        self.assertLess(current, 5 * 1024 * 1024, "Memory usage too high, possible leak detected")
        
    def test_no_memory_leak_with_args(self):
        """Test that logging with arguments doesn't leak memory."""
        tracemalloc.start()
        
        logger = picologging.getLogger("test_args")
        
        # Take initial snapshot
        snapshot1 = tracemalloc.take_snapshot()
        
        # Log many messages with arguments
        for i in range(1000):
            logger.info("test message %s with number %d", "hello" * 10, i)
        
        # Take final snapshot
        snapshot2 = tracemalloc.take_snapshot()
        
        # Check memory growth
        current, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        
        # This is a reasonable threshold - logging 1000 messages shouldn't use more than 5MB
        self.assertLess(current, 5 * 1024 * 1024, "Memory usage too high, possible leak detected")
        
    def test_no_memory_leak_with_stack_info(self):
        """Test that logging with stack_info doesn't leak memory."""
        tracemalloc.start()
        
        logger = picologging.getLogger("test_stack")
        
        # Take initial snapshot
        snapshot1 = tracemalloc.take_snapshot()
        
        # Log messages with stack info (this was the main source of the leak)
        for i in range(100):  # Fewer iterations since stack info is expensive
            logger.info("test message %d", i, stack_info=True)
        
        # Take final snapshot
        snapshot2 = tracemalloc.take_snapshot()
        
        # Check memory growth
        current, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        
        # Stack info is more expensive, but 100 messages shouldn't use more than 10MB
        self.assertLess(current, 10 * 1024 * 1024, "Memory usage too high, possible leak detected")
        
    def test_no_memory_leak_multiple_loggers(self):
        """Test that multiple loggers don't leak memory."""
        tracemalloc.start()
        
        # Create multiple loggers (similar to the original issue)
        loggers = [
            picologging.getLogger("logger1"),
            picologging.getLogger("logger2"), 
            picologging.getLogger("logger3"),
            picologging.getLogger("logger4")
        ]
        
        # Take initial snapshot
        snapshot1 = tracemalloc.take_snapshot()
        
        # Log messages from multiple loggers
        for i in range(250):  # 250 * 4 = 1000 total messages
            for logger in loggers:
                logger.info("message %d from %s", i, logger.name)
        
        # Take final snapshot
        snapshot2 = tracemalloc.take_snapshot()
        
        # Check memory growth
        current, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        
        # Multiple loggers shouldn't significantly increase memory usage
        self.assertLess(current, 5 * 1024 * 1024, "Memory usage too high, possible leak detected")


if __name__ == '__main__':
    unittest.main()