#!/usr/bin/env python3
"""
Test for threading deadlock fix in picologging.
"""
import unittest
import threading
import time
import sys
import picologging


class TestThreadingDeadlock(unittest.TestCase):
    
    def setUp(self):
        # Configure picologging
        picologging.basicConfig(
            level=picologging.INFO, 
            format='[%(name)s] [%(thread)d]: %(message)s'
        )
        
    def test_no_deadlock_multiple_threads(self):
        """Test that multiple threads logging simultaneously don't deadlock."""
        
        # Use a list to track if threads are still running
        threads_running = []
        exception_occurred = []
        
        def log_in_a_loop(name: str, duration: float = 2.0):
            """Log messages for a specified duration."""
            try:
                logger = picologging.getLogger(name)
                start_time = time.time()
                count = 0
                
                while time.time() - start_time < duration:
                    logger.info('log message %d', count)
                    count += 1
                    # Small sleep to allow thread switching
                    time.sleep(0.001)
                    
                threads_running.append(f"{name}: {count} messages")
            except Exception as e:
                exception_occurred.append(f"{name}: {e}")
        
        # Create multiple threads
        threads = []
        for i in range(4):
            thread = threading.Thread(
                name=f"worker-{i}", 
                target=log_in_a_loop, 
                args=[f"worker-{i}", 2.0]
            )
            threads.append(thread)
        
        # Start all threads
        start_time = time.time()
        for thread in threads:
            thread.start()
        
        # Wait for all threads to complete with a timeout
        for thread in threads:
            thread.join(timeout=5.0)  # 5 second timeout
        
        end_time = time.time()
        
        # Check that no exceptions occurred
        self.assertEqual(len(exception_occurred), 0, 
                        f"Exceptions occurred: {exception_occurred}")
        
        # Check that all threads completed (no deadlock)
        self.assertEqual(len(threads_running), 4, 
                        f"Not all threads completed. Running: {threads_running}")
        
        # Check that execution didn't take too long (indicating deadlock)
        execution_time = end_time - start_time
        self.assertLess(execution_time, 4.0, 
                       f"Execution took too long ({execution_time:.2f}s), possible deadlock")
        
        # Verify all threads are no longer alive
        for thread in threads:
            self.assertFalse(thread.is_alive(), 
                           f"Thread {thread.name} is still alive, possible deadlock")
    
    def test_concurrent_logging_different_loggers(self):
        """Test concurrent logging with different logger instances."""
        
        results = {}
        lock = threading.Lock()
        
        def concurrent_logger(logger_name: str, message_count: int = 100):
            """Log messages concurrently."""
            try:
                logger = picologging.getLogger(logger_name)
                for i in range(message_count):
                    logger.info(f'Message {i} from {logger_name}')
                
                with lock:
                    results[logger_name] = message_count
            except Exception as e:
                with lock:
                    results[logger_name] = f"ERROR: {e}"
        
        # Create multiple threads with different loggers
        threads = []
        logger_names = ['logger_a', 'logger_b', 'logger_c', 'logger_d']
        
        for name in logger_names:
            thread = threading.Thread(target=concurrent_logger, args=[name, 50])
            threads.append(thread)
        
        # Start all threads
        for thread in threads:
            thread.start()
        
        # Wait for completion with timeout
        for thread in threads:
            thread.join(timeout=3.0)
        
        # Verify all threads completed successfully
        self.assertEqual(len(results), len(logger_names))
        for name in logger_names:
            self.assertIn(name, results)
            self.assertEqual(results[name], 50, 
                           f"Logger {name} didn't complete: {results[name]}")
    
    def test_handler_acquire_release_threading(self):
        """Test that handler acquire/release methods work correctly in threading."""
        
        logger = picologging.getLogger('test_handler')
        handler = logger.handlers[0] if logger.handlers else None
        
        if handler is None:
            # Create a handler if none exists
            handler = picologging.StreamHandler(sys.stdout)
            logger.addHandler(handler)
        
        results = []
        lock = threading.Lock()
        
        def test_acquire_release():
            """Test acquire and release in a thread."""
            try:
                # This should not deadlock
                handler.acquire()
                time.sleep(0.01)  # Hold lock briefly
                handler.release()
                
                with lock:
                    results.append("success")
            except Exception as e:
                with lock:
                    results.append(f"error: {e}")
        
        # Create multiple threads
        threads = []
        for i in range(5):
            thread = threading.Thread(target=test_acquire_release)
            threads.append(thread)
        
        # Start and wait for threads
        for thread in threads:
            thread.start()
        
        for thread in threads:
            thread.join(timeout=2.0)
        
        # Verify all operations completed successfully
        self.assertEqual(len(results), 5)
        for result in results:
            self.assertEqual(result, "success", f"Handler operation failed: {result}")


if __name__ == '__main__':
    unittest.main()