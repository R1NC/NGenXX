#ifndef NGENXX_SRC_STORE_SQLITE_HXX_
#define NGENXX_SRC_STORE_SQLITE_HXX_

#if defined(__cplusplus)

#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <sqlite3.h>

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Store
    {
        class SQLite
        {
        public:
            /**
             * @brief Initialize SQLite process
             */
            SQLite();
            SQLite(const SQLite &) = delete;
            SQLite &operator=(const SQLite &) = delete;
            SQLite(SQLite &&) = delete;
            SQLite &operator=(SQLite &&) = delete;

            class Connection
            {
            public:
                /**
                 * @warning `Connection` can only be constructed with `SQLite::connect()`
                 */
                Connection() = delete;
                explicit Connection(sqlite3 *db);
                Connection(const Connection &) = delete;
                Connection &operator=(const Connection &) = delete;
                Connection(Connection &&) = delete;
                Connection &operator=(Connection &&) = delete;

                class QueryResult
                {
                public:
                    /**
                     * @warning `QueryResult` can only be constructed with `SQLite::Connection::query()`
                     */
                    QueryResult() = delete;
                    explicit QueryResult(sqlite3_stmt *stmt);
                    QueryResult(const QueryResult &) = delete;
                    QueryResult &operator=(const QueryResult &) = delete;
                    QueryResult(QueryResult &&) = delete;
                    QueryResult &operator=(QueryResult &&) = delete;

                    /**
                     * @brief Read a row from query result
                     * @return Successful or not
                     */
                    [[nodiscard]] bool readRow() const;

                    /**
                     * @brief Read text column data from QueryResult
                     * @param column Column name
                     * @return column data
                     */
                    Any readColumn(const std::string &column) const;

                    /**
                     * @brief Release QueryResult
                     */
                    ~QueryResult();

                private:
                    sqlite3_stmt *stmt{NULL};
                    mutable std::shared_mutex mutex;
                };

                /**
                 * @brief Execute SQL(s)
                 * @param sql SQL
                 * @return Successfull or not.
                 */
                [[nodiscard]] bool execute(const std::string &sql) const;

                /**
                 * @brief Query with a SQL
                 * @param sql SQL
                 * @return A QueryResult, or NULL if execute failed.
                 */
                QueryResult *query(const std::string &sql) const;

                /**
                 * @brief Release DB resource
                 */
                ~Connection();

            private:
                struct sqlite3 *db{NULL};
                mutable std::mutex mutex;
            };

            /**
             * @brief connect DB
             * @param file DB file
             * @return A Connection handle
             */
            std::shared_ptr<Connection> connect(const std::string &file);

            void close(const std::string &file);

            void closeAll();

            /**
             * @brief Release SQLite process
             */
            ~SQLite();

        private:
            std::unordered_map<std::string, std::shared_ptr<Connection>> conns;
            std::mutex mutex;
        };
    }
}

#endif

#endif // NGENXX_SRC_STORE_SQLITE_HXX_
